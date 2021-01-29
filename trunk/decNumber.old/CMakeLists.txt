
cmake_minimum_required(VERSION 2.6)

include_directories("${CMAKE_SOURCE_DIR}/decNumber"  "/usr")



set(decNumber_SRC
decContext.c 
decDouble.c 
decimal128.c 
decimal32.c 
decimal64.c 
decNumber.c 
decPacked.c 
decQuad.c)


set(decNumber_HEADER
decContext.h 
decDouble.h 
decimal128.h 
decimal32.h 
decimal64.h 
decNumberLocal.h 
decNumber.h 
decPacked.h 
decQuad.h 
decSingle.h
ms_stdint.h
)

ADD_LIBRARY (decNumber STATIC ${decNumber_SRC} ${decNumber_HEADER})
