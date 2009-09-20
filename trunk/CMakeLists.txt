cmake_minimum_required(VERSION 2.6)
PROJECT(cpporm)
SUBDIRS (decNumber utf sqlnum example)



add_definitions(-DORM_STATIC_LINKING=1 -DHAVE_BOOST=1 -DORM_HAS_DECNUM=1
-DBOOST_ALL_DYN_LINK=1
-DUSE_BOOST_SERIALIZE=1
-D_CRT_SECURE_NO_DEPRECATE=1
-D_SCL_SECURE_NO_DEPRECATE=1
-D_UNICODE=1
-DUNICODE=1)


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

ENDIF ( WIN32 )
  
#SET( Boost_ADDITIONAL_VERSIONS 1.38 1.38.0 )
# search for Boost version x1.34

SET (Boost_USE_MULTITHREADED 1)
find_package( Boost  COMPONENTS regex asio thread)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS} 
${cpporm_SOURCE_DIR}
${cpporm_SOURCE_DIR}/util
${cpporm_SOURCE_DIR}/utf
${cpporm_SOURCE_DIR}/sqlnum #if ORM_HAS_DECNUM is defined this dir must be included
${cpporm_SOURCE_DIR}/otl
 )
 
 
LINK_DIRECTORIES(${Project_BINARY_DIR}/lib ${Boost_LIBRARY_DIRS} )

