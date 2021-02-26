cmake_minimum_required(VERSION 3.4...3.19)
# cmake tutorial https://gitlab.com/CLIUtils/modern-cmake/-/blob/master/examples/root-dict/CMakeLists.txt

PROJECT(cpporm LANGUAGES CXX C VERSION 0.0.1)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_PLATFORM_INDEPENDENT_CODE ON)


SET (ORM_HAS_DECNUM ON)

message(STATUS "Found ROOT: ${ROOT_VERSION} at ${ROOT_DIR}")
message(STATUS "Original flags: ${ROOT_CXX_FLAGS}")
message(STATUS "Original definitions: ${ROOT_DEFINITIONS}")
message(STATUS "Original exe flags: ${ROOT_EXE_LINKER_FLAGS}")

message(STATUS "Using DECNUM: ${ORM_HAS_DECNUM}")




if(ORM_HAS_DECNUM)
    add_definitions(-DORM_STATIC_LINKING=1 -DHAVE_BOOST=1 -DORM_HAS_DECNUM=1
    -DBOOST_ALL_DYN_LINK=1
    -DUSE_BOOST_SERIALIZE=1
    -D_CRT_SECURE_NO_DEPRECATE=1
    -D_SCL_SECURE_NO_DEPRECATE=1
    -D_UNICODE=1
    -DUNICODE=1)
ELSE(ORM_HAS_DECNUM)
    add_definitions(-DORM_STATIC_LINKING=1 -DHAVE_BOOST=1
    -DBOOST_ALL_DYN_LINK=1
    -DUSE_BOOST_SERIALIZE=1
    -D_CRT_SECURE_NO_DEPRECATE=1
    -D_SCL_SECURE_NO_DEPRECATE=1
    -D_UNICODE=1
    -DUNICODE=1)
ENDIF (ORM_HAS_DECNUM)


IF ( WIN32 )
    add_definitions (-D_WIN32_WINNT=0x0501)
    add_definitions (-DWIN32_LEAN_AND_MEAN=1)
    SET (ORM_PLAFORM_ODBCLIB odbc32.lib)	

    # Assumes BOOST_ROOT and BOOST_THREAD_* are set in your path statement.
    # If not then either add them to your path statement or edit the following
    # lines putting in the absolute paths to the boost includes and libraries.
    #
    # I typically have boostroot set in my command prompt so cmake-gui needs to be
    # started from within the command prompt where BOOST_ROOT is defined
    # BOOST_ROOT must point to the start of the boost directory
    SET ( Boost_Root    $ENV{BOOST_ROOT} )
ELSE ( WIN32 )
    add_definitions(-DOTL_IODBC_BSD=1)
ENDIF ( WIN32 )

find_package(ODBC REQUIRED)
message(STATUS "Using ODBC libraries: ${ODBC_LIBRARY}")

#SET( Boost_ADDITIONAL_VERSIONS 1.38 1.38.0 )
# search for Boost version x1.34

# set(Boost_USE_STATIC_LIBS ON)
SET (Boost_USE_MULTITHREADED 1)
find_package( Boost  COMPONENTS regex thread serialization)

set (myinclude_dirs)
list (APPEND myinclude_dirs ${Boost_INCLUDE_DIRS} )
list (APPEND myinclude_dirs ${cpporm_SOURCE_DIR} )
list (APPEND myinclude_dirs ${cpporm_SOURCE_DIR}/util )
list (APPEND myinclude_dirs ${cpporm_SOURCE_DIR}/utf )
list (APPEND myinclude_dirs ${cpporm_SOURCE_DIR}/otl )
if (ORM_HAS_DECNUM)
    list (APPEND myinclude_dirs ${cpporm_SOURCE_DIR}/sqlnum ) #if ORM_HAS_DECNUM is defined this dir must be included
    list (APPEND myinclude_dirs ${cpporm_SOURCE_DIR}/decNumber/include ) 
endif (ORM_HAS_DECNUM)

INCLUDE_DIRECTORIES(${myinclude_dirs} ${ODBC_INCLUDE_DIRS} )
 
 
LINK_DIRECTORIES(${Project_BINARY_DIR}/lib ${Boost_LIBRARY_DIRS} )

if (ORM_HAS_DECNUM)
  set (FORCE_DECNUMBERLIB_NM decNumber)
  add_subdirectory (decNumber)
  add_subdirectory (utf)
  add_subdirectory (sqlnum)
  add_subdirectory (example)
ELSE (ORM_HAS_DECNUM)
  add_subdirectory (utf)
  add_subdirectory (example)
ENDIF (ORM_HAS_DECNUM)
