cmake_minimum_required(VERSION 3.4...3.19)
set(sqlnum_SRC dbt_money.cpp)

set(sqlnum_HEADER dbt_money.hpp)

ADD_LIBRARY (sqlnum STATIC ${sqlnum_SRC} ${sqlnum_HEADER})
