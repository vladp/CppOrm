cmake_minimum_required(VERSION 3.4...3.19)


set(utf_SRC 
ConvertUTF.c)


set(utf_HEADER
ConvertUTF.h 
UtfConverter.h)

add_library (utf STATIC ${utf_SRC} ${utf_HEADER})
# All users of this library will need at least C++11
target_compile_features(utf PUBLIC cxx_std_11)
