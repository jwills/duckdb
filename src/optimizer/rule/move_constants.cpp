#include "duckdb/optimizer/rule/move_constants.hpp"

#include "duckdb/common/exception.hpp"
#include "duckdb/common/value_operations/value_operations.hpp"
#include "duckdb/planner/expression/bound_comparison_expression.hpp"
#include "duckdb/planner/expression/bound_constant_expression.hpp"
#include "duckdb/planner/expression/bound_function_expression.hpp"

namespace duckdb {

MoveConstantsRule::MoveConstantsRule(ExpressionRewriter &rewriter) : Rule(rewriter) {
	auto op = make_unique<ComparisonExpressionMatcher>();
	op->matchers.push_back(make_unique<ConstantExpressionMatcher>());
	op->policy = SetMatcher::Policy::UNORDERED;

	auto arithmetic = make_unique<FunctionExpressionMatcher>();
	// we handle multiplication, addition and subtraction because those are "easy"
	// integer division makes the division case difficult
	// e.g. [x / 2 = 3] means [x = 6 OR x = 7] because of truncation -> no clean rewrite rules
	arithmetic->function = make_unique<ManyFunctionMatcher>(unordered_set<string> {"+", "-", "*"});
	// we match only on integral numeric types
	arithmetic->type = make_unique<IntegerTypeMatcher>();
	arithmetic->matchers.push_back(make_unique<ConstantExpressionMatcher>());
	arithmetic->matchers.push_back(make_unique<ExpressionMatcher>());
	arithmetic->policy = SetMatcher::Policy::SOME;
	op->matchers.push_back(move(arithmetic));
	root = move(op);
}

unique_ptr<Expression> MoveConstantsRule::Apply(LogicalOperator &op, vector<Expression *> &bindings, bool &changes_made,
                                                bool is_root) {
	auto comparison = (BoundComparisonExpression *)bindings[0];
	auto outer_constant = (BoundConstantExpression *)bindings[1];
	auto arithmetic = (BoundFunctionExpression *)bindings[2];
	auto inner_constant = (BoundConstantExpression *)bindings[3];
	if (!TypeIsIntegral(arithmetic->return_type.InternalType())) {
		return nullptr;
	}
	if (inner_constant->value.IsNull() || outer_constant->value.IsNull()) {
		return make_unique<BoundConstantExpression>(Value(comparison->return_type));
	}
	auto &constant_type = outer_constant->return_type;
	hugeint_t outer_value = IntegralValue::Get(outer_constant->value);
	hugeint_t inner_value = IntegralValue::Get(inner_constant->value);

	idx_t arithmetic_child_index = arithmetic->children[0].get() == inner_constant ? 1 : 0;
	auto &op_type = arithmetic->function.name;
	if (op_type == "+") {
		// [x + 1 COMP 10] OR [1 + x COMP 10]
		// order does not matter in addition:
		// simply change right side to 10-1 (outer_constant - inner_constant)
		if (!Hugeint::SubtractInPlace(outer_value, inner_value)) {
			return nullptr;
		}
		auto result_value = Value::HUGEINT(outer_value);
		if (!result_value.TryCastAs(constant_type)) {
			// if the cast is not possible then the comparison is not possible
			// for example, if we have x + 5 = 3, where x is an unsigned number, we will get x = -2
			// since this is not possible we can remove the entire branch here
			return ExpressionRewriter::ConstantOrNull(move(arithmetic->children[arithmetic_child_index]),
			                                          Value::BOOLEAN(false));
		}
		outer_constant->value = move(result_value);
	} else if (op_type == "-") {
		// [x - 1 COMP 10] O R [1 - x COMP 10]
		// order matters in subtraction:
		if (arithmetic_child_index == 0) {
			// [x - 1 COMP 10]
			// change right side to 10+1 (outer_constant + inner_constant)
			if (!Hugeint::AddInPlace(outer_value, inner_value)) {
				return nullptr;
			}
			auto result_value = Value::HUGEINT(outer_value);
			if (!result_value.TryCastAs(constant_type)) {
				// if the cast is not possible then the comparison is not possible
				return ExpressionRewriter::ConstantOrNull(move(arithmetic->children[arithmetic_child_index]),
				                                          Value::BOOLEAN(false));
			}
			outer_constant->value = move(result_value);
		} else {
			// [1 - x COMP 10]
			// change right side to 1-10=-9
			if (!Hugeint::SubtractInPlace(inner_value, outer_value)) {
				return nullptr;
			}
			auto result_value = Value::HUGEINT(inner_value);
			if (!result_value.TryCastAs(constant_type)) {
				// if the cast is not possible then the comparison is not possible
				return ExpressionRewriter::ConstantOrNull(move(arithmetic->children[arithmetic_child_index]),
				                                          Value::BOOLEAN(false));
			}
			outer_constant->value = move(result_value);
			// in this case, we should also flip the comparison
			// e.g. if we have [4 - x < 2] then we should have [x > 2]
			comparison->type = FlipComparisionExpression(comparison->type);
		}
	} else {
		D_ASSERT(op_type == "*");
		// [x * 2 COMP 10] OR [2 * x COMP 10]
		// order does not matter in multiplication:
		// change right side to 10/2 (outer_constant / inner_constant)
		// but ONLY if outer_constant is cleanly divisible by the inner_constant
		if (inner_constant->value == 0) {
			// x * 0, the result is either 0 or NULL
			// thus the final result will be either [TRUE, FALSE] or [NULL], depending
			// on if 0 matches the comparison criteria with the RHS
			bool outer_is_null = outer_constant->value == 0;
			return ExpressionRewriter::ConstantOrNull(move(arithmetic->children[arithmetic_child_index]),
			                                          Value::BOOLEAN(outer_is_null));
		}
		if (outer_value % inner_value != 0) {
			// not cleanly divisible, the result will be either FALSE or NULL
			return ExpressionRewriter::ConstantOrNull(move(arithmetic->children[arithmetic_child_index]),
			                                          Value::BOOLEAN(false));
		}
		if (inner_value < 0) {
			// multiply by negative value, need to flip expression
			comparison->type = FlipComparisionExpression(comparison->type);
		}
		// else divide the RHS by the LHS
		// we need to do a range check on the cast even though we do a division
		// because e.g. -128 / -1 = 128, which is out of range
		auto result_value = Value::HUGEINT(outer_value / inner_value);
		if (!result_value.TryCastAs(constant_type)) {
			return ExpressionRewriter::ConstantOrNull(move(arithmetic->children[arithmetic_child_index]),
			                                          Value::BOOLEAN(false));
		}
		outer_constant->value = move(result_value);
	}
	// replace left side with x
	// first extract x from the arithmetic expression
	auto arithmetic_child = move(arithmetic->children[arithmetic_child_index]);
	// then place in the comparison
	if (comparison->left.get() == outer_constant) {
		comparison->right = move(arithmetic_child);
	} else {
		comparison->left = move(arithmetic_child);
	}
	changes_made = true;
	return nullptr;
}

} // namespace duckdb
