
cmake_minimum_required(VERSION 2.6)

set(sqlnum_SRC 
dbt_money.cpp)


set(sqlnum_HEADER
dbt_money.hpp)

ADD_LIBRARY (sqlnum STATIC ${sqlnum_SRC} ${sqlnum_HEADER})