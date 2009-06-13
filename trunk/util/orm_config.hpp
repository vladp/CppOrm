/* 
		
  There is no explicit license for this file
  Only thing I ask is keep the names of the 
  author/contributors in this file


  Author: Vladislav Papayan
  Contributors:



  Change History:
          6-10-09  vpapayan  Pulled out of my private archives for initial checking 

	$id$


  Purpose:
      Define various macros especially needed for library exports on windows platforms

*/

#ifndef ORM_CONFIG_HPP__
#define ORM_CONFIG_HPP__

/* do not include boost stuff
if the file that is including this header
is being compiled as 'C' (not c++) )
*/
#ifdef __cplusplus
#include <boost/config.hpp>
#endif


#if defined(_DEBUG) && defined(WIN32)
/*
   #ifndef new
      #define new MYDEBUG_NEW
   #endif      
*/   
#endif


// http://www.codeguru.com/cpp/misc/misc/compilerandpre-compiler/article.php/c14797

#define PRAGMA_STR1(x)  #x
#define PRAGMA_STR2(x)  PRAGMA_STR1 (x)
#define NOTE(x)  message (__FILE__ "(" PRAGMA_STR2(__LINE__) ") :   -NOTE- " #x)
#define NOTE_wARG(x,y)  message   (__FILE__ "(" PRAGMA_STR2(__LINE__) ") :   -NOTE- " #x PRAGMA_STR2(y))
   
// #pragma NOTE (TO DO - add error checking of inputs)
// will show up at compile time as a warning   




#ifdef _MSC_VER

	#ifdef ORM_COMMON_EXPORTS
		#define ORM_COMMON_API __declspec(dllexport)
	#elif defined ORM_STATIC_LINKING
		#define ORM_COMMON_API
	#else
		#define ORM_COMMON_API __declspec(dllimport)
	#endif


	#ifdef ORM_STATIC_LINKING
		#define ORM_SERVICE_API 
	#else
		#define ORM_SERVICE_API __declspec(dllexport)
	#endif

	#ifdef ORM_STATIC_LINKING
		#define ORM_PLUGIN_API
	#else
		#define ORM_PLUGIN_API __declspec(dllexport)
	#endif

	#ifdef ORM_PLATFORM_EXPORTS
		#define ORM_PLATFORM_API __declspec(dllexport)
	#elif defined ORM_STATIC_LINKING
		#define ORM_PLATFORM_API
	#else
		#define ORM_PLATFORM_API __declspec(dllimport)
	#endif

	#ifdef ORM_SERVER_EXPORTS
		#define ORM_SERVER_API __declspec(dllexport)
	#elif defined ORM_STATIC_LINKING
		#define ORM_SERVER_API
	#else
		#define ORM_SERVER_API __declspec(dllimport)
	#endif

	/*
	Verify correctness of the ORM_STATIC_LINKING setup
	*/
	#ifdef ORM_STATIC_LINKING
		#ifdef _USRDLL
			#error Need to be compiled as a static library for ORM_STATIC_LINKING
		#endif
	#endif

#endif // _MSC_VER

// http://predef.sourceforge.net/prearch.html#sec6

#if defined(_M_X86) || defined (__i386) || defined (__i386__)|| defined (__i486__)|| defined (__i586__)|| defined (__i686__) || defined(_X86_) || defined (__I86__) || defined (__x86__) || defined (_M_IX86)
      #define ORM_IS_x86  x86
      #define ORM_CPU_ARCH_NUM 86
      #define ORM_CPU_ARCH_NM x86
//      #pragma NOTE(MY COMPILE DETECTED CPU x86)
#elif defined(__sparc__) || defined(__sparcv9)
      #undef ORM_IS_x86  x86
      #define ORM_CPU_ARCH_NUM 90
      #define ORM_CPU_ARCH_NM v90
#else
#pragma NOTE(MY COMPILE DID NOT DETECT CPU)
   #error cannot figure out CPU type
#endif      


#if !defined (ORM_BLDTYP)
#if defined (_DEBUG)
#define ORM_BLDTYP 1
#define ORM_BLDLETTER d
#endif
#endif






//otherwise assumed x64 we should probably if some of x64 macros are defined otherwise error




/* I would like to define MACRO_append_CPU_BLDTYP
by appending CPU type macro and the Build type to
the argument.  However at least in VC 9 -- those
macros are not expanded when used within a macor

So I had to essentially hardcode varius cpu types
and build type

*/

#if ORM_CPU_ARCH_NUM == 86
   #if defined (_DEBUG)
      #define MACRO_append_CPU_BLDTYP(arg1) \
      arg1 ## _x86d
   #else
      #define MACRO_append_CPU_BLDTYP(arg1) \
      arg1 ## _x86
   #endif
#elif ORM_CPU_ARCH_NUM == 64
   #if defined (_DEBUG)
      #define MACRO_append_CPU_BLDTYP(arg1) \
      arg1 ## _x64d
   #else
      #define MACRO_append_CPU_BLDTYP(arg1) \
      arg1 ## _x64
   #endif
#elif ORM_CPU_ARCH_NUM == 90 //sparc
   #if defined (_DEBUG)
      #define MACRO_append_CPU_BLDTYP(arg1) \
      arg1 ## _v90d
   #else
      #define MACRO_append_CPU_BLDTYP(arg1) \
      arg1 ## _v90
   #endif
#else
   #error CPU IS NOT DEFINED
#endif

//put that in every class definition as the first line
#define CLASS_HEADER_M(classnm)
//put that in every function entry
#define FUNCTION_ENTRY_M(funcnm) \
   std::string func_entry_nm(#funcnm);

#endif //end of file


