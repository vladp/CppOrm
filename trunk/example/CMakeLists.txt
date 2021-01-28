
cmake_minimum_required(VERSION 2.6)

set(ex1_SRC 
ex1.cpp)


set(ex1_HEADER )

ADD_EXECUTABLE (ex1 ${ex1_SRC} )

if(ORM_HAS_DECNUM)
TARGET_LINK_LIBRARIES(ex1 utf decNumber sqlnum odbc32 ${Boost_LIBRARIES} ${ORM_PLATFORM_ODBCLIB})
else(ORM_HAS_DECNUM)
TARGET_LINK_LIBRARIES(ex1 utf odbc32 ${Boost_LIBRARIES} ${ORM_PLATFORM_ODBCLIB})
endif(ORM_HAS_DECNUM)