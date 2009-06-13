
cmake_minimum_required(VERSION 2.6)

set(utf_SRC 
ConvertUTF.c)


set(utf_HEADER
ConvertUTF.h 
utfconverter.h)

ADD_LIBRARY (utf STATIC ${utf_SRC} ${utf_HEADER})