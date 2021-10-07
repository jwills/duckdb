import pytest
import duckdb
import pandas as pd
import numpy as np

def assert_create(internal_data, expected_result, data_type):
    conn = duckdb.connect()
    df_in = pd.DataFrame(data=internal_data, dtype=data_type)

    conn.execute("CREATE TABLE t AS SELECT * FROM df_in")

    result = conn.execute("SELECT * FROM t").fetchall()
    assert result == expected_result

def assert_create_register(internal_data, expected_result, data_type):
    conn = duckdb.connect()
    df_in = pd.DataFrame(data=internal_data, dtype=data_type)
    conn.register("dataframe", df_in)
    conn.execute("CREATE TABLE t AS SELECT * FROM dataframe")

    result = conn.execute("SELECT * FROM t").fetchall()
    assert result == expected_result

class TestCreateTableFromPandas(object):

    def test_integer_create_table(self, duckdb_cursor):
        #FIXME: This should work with other data types e.g., int8...
        data_types = ['Int8','Int16','Int32','Int64']
        internal_data = [1,2,3,4]
        expected_result = [(1,), (2,), (3,), (4,)]
        for data_type in data_types:
            print(data_type)
            assert_create_register(internal_data,expected_result,data_type)
            assert_create(internal_data,expected_result,data_type)

    #FIXME: Also test other data types