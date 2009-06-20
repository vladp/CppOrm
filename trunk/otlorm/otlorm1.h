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
      Active Record / ORM (Object Relational Mapping) implementation for C++ using
      OTL for DB access and boost.

      The basic concept:
        Declare a CPP structure repsenting a table row.
        Then the ORM will generate (mostly at compile time) various Database operators
        for that Table (select/insert/delete/update)


        Every row then can be read into a table (that is based on STL vector )
        Then the rows can be manipualted in memory as any vector


        The CPP structure representing a table, must be declare in a special way using
        provided macros (see examples)

        Those macros use static template arguments and initialize several
        mutex protected static lists that contain an 'expanded' description for each
        field with the structure.
        Then ORM methods access those static structures to do operations on each field.
        The above is basically a rudimentory implementation of Reflections for C++

        Inheritance is NOT supported  (that is if you define Table 1 and table 2 has
        just one extra field, you can not really derive table 2 from 1 and expect the
        ORM to figure out the composite row representation for both tables)

        The magic macros are
        DECL_OTL_FIELD
        DECL_OTL_ROW_HPP
        DECL_OTL_ROW_CPP


        again, see examples of how they are used

*/


#ifndef otl_orm_extension_proposal1_h__
#define otl_orm_extension_proposal1_h__


/* if otl is not included in precompiled headers */
#if !defined (OTLEXPR1_IN_PRECOMPILED_HEADERS)

#include <util/orm_config.hpp>

#if defined(BOOST_VERSION) || defined(HAVE_BOOST) || defined(USE_BOOST)
#define OTLORM1_BOOST 1
#define OTLORM1_BOOST_THREAD 1
#else
#error cannot work without boost
#endif




//this must be
//be included before winsock2
#include <boost/asio.hpp>


#if defined _MSC_VER
   #include <winsock2.h>
   #include <windows.h>
#endif


//#define BOOST_BIND_ENABLE_CDECL 1
//#define BOOST_MEM_FN_ENABLE_CDECL 1

//this cannot be included in stdafx
//because depending on what library I build
//I changed some of the pound defines there
//#include <setrak_config.hpp>



#include <strstream>

#include <boost/config.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>



/* VP
#if defined(_DEBUG) && defined (WIN32)
   #ifndef new
      #define new MYDEBUG_NEW
   #endif      
#endif
*/

// -------------------------------------
//
// Headers that allow to overload NEW
// must come after this line
//
//--------------------------------------


#include <vector>
#include <map>
// vp fixup stdafx
#include <set>
#include <queue>
#include <algorithm>
#include <iostream>



#include <boost/bind.hpp>
//vp fixup stdafx
#include <boost/shared_ptr.hpp>
//vp fixup stdafx
#include <boost/utility.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/operators.hpp>
#include <boost/cstdint.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/tokenizer.hpp>

//I use it to get process id
#include <boost/interprocess/managed_shared_memory.hpp>


#include <boost/any.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/int.hpp>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp> //add to stdafx.h
#include <boost/serialization/export.hpp> //add to stdafx.h

#include <boost/serialization/set.hpp> //add to stdafx.h
#include <boost/serialization/vector.hpp> //add to stdafx.h
#include <boost/serialization/list.hpp> //add to stdafx.h
#include <boost/serialization/deque.hpp> //add to stdafx.h



#include <boost/serialization/nvp.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>










//otl stuff start


#ifdef _DEBUG
/*
      unsigned int my_trace_level=
        0x1 | // 1st level of tracing
        0x2 | // 2nd level of tracing
        0x4 | // 3rd level of tracing
        0x8 | // 4th level of tracing
        0x10| // 5th level of tracing
        0x20; // 6th level of tracing
     // each level of tracing is represented by its own bit, 
     // so levels of tracing can be combined in an arbitrary order.
*/     
      #define OTL_TRACE_LEVEL 0x1
     //#define OTL_TRACE_LEVEL 0xFF
        // enables OTL tracing, and use my_trace_level 
        // as a trace control variable.

     //#define OTL_TRACE_STREAM std::cerr
     
     #define OTL_TRACE_ENABLE_STREAM_LABELS 1
     #define OTL_TRACE_STREAM cout
     

     #define OTL_TRACE_LINE_PREFIX "=otl=> " 
        // redefines the default OTL trace line prefix. 
        // This #define is optional

     #define OTL_TRACE_LINE_SUFFIX std::endl 
        // redefines the default OTL trace line suffix. 
        // This #define is optional

#endif


//#define OTL_STL 1
//cannot use OTL_STL and UNICODE
#define OTL_ADD_NULL_TERMINATOR_TO_STRING_SIZE
#define OTL_ODBC_POSTGRESQL
//new define for proper RPC (return rows count)
#define OTL_ODBC_ALTERNATE_RPC 1
//#define OTL_ODBC_SQL_EXTENDED_FETCH_ON 1


#define OTL_BIND_VAR_STRICT_TYPE_CHECKING_ON
#define OTL_EXPLICIT_NAMESPACES
#define OTL_ANSI_CPP
#define OTL_UNCAUGHT_EXCEPTION_ON // enable safe exception handling / error 
                                  // recovery.

#if defined(__BORLANDC__)
#define OTL_BIGINT __int64 // Enabling BC++ 64-bit integers
#elif !defined(_MSC_VER)
#define OTL_BIGINT long long // Enabling G++ 64-bit integers
#else
#define OTL_BIGINT __int64 // Enabling VC++ 64-bit integers
#endif


#define OTL_VALUE_TEMPLATE_ON // Turn on otl_value<T>
//must be combined with user defined string class
#define OTL_USER_DEFINED_STRING_CLASS_ON

//#define OTL_UNICODE 1// Enable Unicode OTL for ODBC



#if defined OTL_UNICODE

//must be defined because OTL_val template is on
//and OTL_STL is off

//CANNOT BE of theh same type as the 
//OTL_UNICODE_STRING_TYPE
#define USER_DEFINED_STRING_CLASS std::string

#if defined(__GNUC__)

namespace std{
   typedef unsigned short unicode_char;
   typedef basic_string<unicode_char> unicode_string;
}

#define OTL_UNICODE_CHAR_TYPE unicode_char
#define OTL_UNICODE_STRING_TYPE unicode_string

#else
#define OTL_UNICODE_CHAR_TYPE wchar_t
#define OTL_UNICODE_STRING_TYPE wstring
#endif

//ASSUME that OTL_UNICODE means doublebyte characters
//and that THE database driver convers from the
//double byte (UTF-16) to whatever the database needs
//(in my case it is UTF-8)

#define OTL_ODBC_STRING_TO_TIMESTAMP(str,tm)            \
{                                                       \
  swscanf(str,                                           \
         L"%04d-%02d-%02d %02d:%02d:%02d.%06ld%hd",      \
         &tm.year,                                      \
         &tm.month,                                     \
         &tm.day,                                       \
         &tm.hour,                                      \
         &tm.minute,                                    \
         &tm.second,                                    \
         &tm.fraction,                                  \
         &tm.tz_hour);                                  \
}

#define OTL_ODBC_TIMESTAMP_TO_STRING(tm,str)            \
{                                                       \
  swprintf(str,100,                                     \
         L"%04d-%02d-%02d %02d:%02d:%02d.%06ld %+hd",    \
          tm.year,                                      \
          tm.month,                                     \
          tm.day,                                       \
          tm.hour,                                      \
          tm.minute,                                    \
          tm.second,                                    \
          tm.fraction,                                  \
          tm.tz_hour);                                  \
}



#else //not double byte unicode

//must be defined because OTL_value template is on
//and otl_stl is off
#define USER_DEFINED_STRING_CLASS std::string

//for non unicode version towidechar
#define ORM_STDSTRING_TO_OTL_STRING (varnm)     
#define ORM_OTL_STRING_TO_STDSTRING (varnm) 

#define OTL_ODBC_STRING_TO_TIMESTAMP(str,tm)            \
{                                                       \
  sscanf(str,                                           \
         "%04d-%02d-%02d %02d:%02d:%02d.%06ld%hd",      \
         &tm.year,                                      \
         &tm.month,                                     \
         &tm.day,                                       \
         &tm.hour,                                      \
         &tm.minute,                                    \
         &tm.second,                                    \
         &tm.fraction,                                  \
         &tm.tz_hour);                                  \
}

#define OTL_ODBC_TIMESTAMP_TO_STRING(tm,str)            \
{                                                       \
  sprintf(str,                                          \
         "%04d-%02d-%02d %02d:%02d:%02d.%06ld %+hd",    \
          tm.year,                                      \
          tm.month,                                     \
          tm.day,                                       \
          tm.hour,                                      \
          tm.minute,                                    \
          tm.second,                                    \
          tm.fraction,                                  \
          tm.tz_hour);                                  \
}

#endif

#define OTL_ODBC_TIME_ZONE



//#include <setrak_servers_common/priv/otlv4.h>
#include <otl/otlv4.h>

//end of OTL stuff

/*

#if defined (ORM_HAS_DECNUM)
extern "C"
{
   #include <decNumber/decQuad.h>
   #include <decNumber/decimal64.c>
}
#endif

*/


#endif //OTLEXPR1_IN_PRECOMPILED_HEADERS

/*
since I constantly switch for testing between
otl unicode and no  otl unicode
I want to define a String type that I can use
that would 'follow' the OTL string type
this will let me easily declare field types
that are 'String' in double byte(UTF-16) and
still UTF-8 (multi-byte) when unicode is off

OTL's unicode is really UTF-16
so if you use 'Unicode' version of the ODBC driver,
it means that regardless of your database's encoding,
the 'unicode' version of ODBC driver will convert
everything to UTF-16 (because that the ODBC's standard
for handling unicode charactres.
That's why OTL assumes 'UTF-16' if you define unicode.

You must NOT define OTL_UNICODE or use ODBC Unicode
driver, if you want to deal with UTF-8 encoded text.
That means couple of things:
a) your database if UTF-8 encoded
b) your database driver (ODBC or OCI) is *NOT* unicode
instead it is just plain 'ASCII'
This way you will receive strings just as they 
are stored in the database as stream of bytes.
Then it is YOUR job to understand that String lengths
reported by OTL, or the underlying ASCII drivers (ODBC or OCI)
are 'byte' length. 
Therefore you must use special routines (widely available on the net)
that deal with 'UTF-8' either directly, or first convert
to 'UTF-16' and then deal with the UTF-16 using C++ standard
libraries (and then convert back to UTF-8 if needed)
*/





//if I declared something as std::string
//and OTL_UNICODE is enabled, I must convert the
//my STD::string (which is either ascii or UTF-8
//to wstring

//this must be after otl include
#include "UtfConverter.h"
#include "ConvertUTF.h"



#if defined (ORM_HAS_DECNUM)
   #define ORM_HAS_CMONEY
   #include "dbt_money.hpp"
#endif


/* orm varchar class 
   this let's me to switch
   between various string types
   
   */
   

#if defined (OTL_UNICODE_CHAR_TYPE) && defined (OTL_UNICODE_STRING_TYPE)
   typedef OTL_UNICODE_CHAR_TYPE csqlchar_t; 
   typedef OTL_UNICODE_STRING_TYPE csqlstring_t ;
#else   
   typedef  char csqlchar_t; 
   typedef  std::string csqlstring_t;
#endif



#if defined(OTL_UNICODE)
#define ORM_STDSTRING_TO_OTL_STRING(varnm) \
   UtfConverter::FromUtf8(varnm) 
    
#define ORM_OTL_STRING_TO_STDSTRING(varnm) \
    UtfConverter::ToUtf8(varnm)

//---------------------------------------
//
// when OTL_UNICODE Is defined 
// OTL expects all strings to be widechar strings
// so we need to ascii strings to convert to widechar strings
//    
//---------------------------------------
inline    
odbc::otl_stream&
operator >> (odbc::otl_stream& s,otl_value<std::string>& v)
{  
   OTL_UNICODE_STRING_TYPE tmp;
   s>>tmp;
   v=UtfConverter::ToUtf8(tmp);
   
   return s;
}



#endif

inline
void toutf8 (const csqlstring_t& source,std::string& res)
{
   #if defined(OTL_UNICODE)
      res=UtfConverter::ToUtf8(source);
   #else
      res=source;
   #endif

}

#if defined(OTL_UNICODE)
inline
csqlstring_t to_csqlstring (const std::string& source)
{
   csqlstring_t res;   
   res=UtfConverter::FromUtf8(source);   
   return res;
}
#else
//we return reference here for speed
//while in the unicode version we return a new str
inline
const csqlstring_t& to_csqlstring (const std::string& source)
{
   return source;
}
#endif


inline
otl_datetime   to_otldatetime(const boost::posix_time::ptime& pt,
                              int tz_minutes_offset=0)
{
   const boost::gregorian::date& dt(pt.date());
   const boost::posix_time::time_duration& td(pt.time_of_day());

   
   otl_datetime tm;
      
      
  tm.year=dt.year();
  tm.month=dt.month();
  tm.day=dt.day();
  tm.hour=td.hours();
  tm.minute=td.minutes();
  tm.second=td.seconds();
  tm.frac_precision=6; // microseconds
  tm.fraction=0;
  int sign= tz_minutes_offset > 0 ? 1:-1;
  tm.tz_hour=tz_minutes_offset%60; // time zone offset in hours
  tm.tz_minute= /* sign* */
            (abs(tz_minutes_offset)-( abs(tm.tz_hour*60)));               

   
   return tm;
}                              


template < typename T>
inline
void assign_random_val(otl_value<T>& in)
{
   in=boost::lexical_cast<T>(std::time(0));
}

template <>
inline
void assign_random_val(otl_value<otl_datetime>& in)
{
   in=to_otldatetime(boost::posix_time::second_clock::universal_time());   
}

#ifdef ORM_HAS_CMONEY
template <>
inline
void assign_random_val(otl_value<cmoney_t>& in)
{
   in=cmoney_t("33222112121.212");   
}


#endif




#define VARCHAR_MAXLEN 65535




//I use ODBC by default, this can be a 
//pound define from otl
using namespace odbc;


namespace otlorm1
{


inline
void orm_exec (otl_stream& s)
{
   s.flush();
}






//otl does not provide operator that allows
//to print otl_datetime to wostream
//so we will just copy the version OTL has
//and make it support wostream 
inline STD_NAMESPACE_PREFIX wostream& operator<<(
 STD_NAMESPACE_PREFIX wostream& s, 
 const otl_value<otl_datetime>& var)
{
 if(var.is_null())
   s<<L"NULL";
 else{
   s<<var.v.month<<L"/"<<var.v.day<<L"/"<<var.v.year<<L" "
    <<var.v.hour<<L":"<<var.v.minute<<L":"<<var.v.second;
 }
 return s;
}



//OTL does not provide support for
//otl_value<T> when used with wstream
//so we add it here

template <typename T>
std::wostream& operator<<(std::wostream& o, const otl_value<T>& in)
{
   o<<in.v;
   return o;
}


//I need to be able to print out 
// a wstring into cout
//this is more for convinience then necessity
//In reality I should just not do that
//but I will basically convert the string
//into utf-8 and print it out
//hoping that console on windows is set
//to 6001 code page
//use   chcp  6001 in cosole window to set
//the code page
inline STD_NAMESPACE_PREFIX ostream& operator<<(
 STD_NAMESPACE_PREFIX ostream& s, 
 const otl_value<STD_NAMESPACE_PREFIX wstring>& var)
{
 if(var.is_null())
   s<<"NULL";
 else{
   s<<UtfConverter::ToUtf8(var.v);
 }
 return s;
}







#ifdef ORM_HAS_CMONEY   
     
inline 
otl_stream& operator >> (otl_stream& s,cmoney_t& m)
{
   csqlstring_t tmp;
   s>>tmp;
#if defined(OTL_UNICODE)   
   m.from_wstr(tmp);
#else   
   m.from_str(tmp);
#endif   
   return s;
}


inline 
otl_stream& operator >> (otl_stream& s,otl_value<cmoney_t>& m)
{
   
   otl_value<csqlstring_t> tmpval;
   s>>tmpval;
   if (tmpval.is_null()==false)
   {
      //if not null
#if defined(OTL_UNICODE)
      m.v.from_wstr(tmpval.v);
#else            
      m.v.from_str(tmpval.v);
#endif      
      
   }      
   m.ind=tmpval.ind;
   return s;
}




inline 
otl_stream& operator << (otl_stream& s,const cmoney_t& m)
{   
#if defined (OTL_UNICODE)
   s<<m.as_wstr();
#else   
   s<<m.as_str();
#endif   
   return s;
}


inline 
otl_stream& operator << (otl_stream& s, const otl_value<cmoney_t>& m)
{
   if (m.is_null())
   {
      //this supposed to generate a null
      s<<m;
   }

#if defined(OTL_UNICODE)
   s<<m.v.as_wstr();
#else   
   s<<m.v.as_str();      
#endif   
   return s;
}


#endif










#ifdef BOOST_SERIALIZATION_SERIALIZATION_HPP
template <typename Archive>
struct functor_serialize_otl_val
{
functor_serialize_otl_val (Archive& ar )
:ref_ar(ar)
{}

   void operator () (otl_value<float>& in)
   {
      operator&(ref_s,in);
      return ref_s;
   }
   
   void operator () (otl_value<OTL_BIGINT>& in)
   {
      operator&(ref_s,in);
      return ref_s;
   }

      
   void operator () (otl_value<std::string>& in)
   {
      operator&(ref_s,in);;
      return ref_s;
   }

   void operator () (otl_value<otl_datetime>& in)
   {
      operator&(ref_s,in);;
      return ref_s;
   }

   void operator () (otl_value<double>& in)
   {
      operator&(ref_s,in);;
      return ref_s;
   }
   void returnres(void ){return;};
};
#endif //BOOST_SERIALIZATION_SERIALIZATION_HPP


/////////////////////////////////////////////
// functions that AUGMENT OTL when needed
//




const int orm_var_decimal=1001;
const int orm_var_timestamp=1002;
const int orm_var_timestamp_tz=1003;
inline
const char* orm_db_type_name (const int ftype,const int len=0)
{
   switch (ftype)
   {
      case otl_var_bigint:
               {
                  static char* res="bigint";
                  return res;
               }
               break;
      case otl_var_int:
               {
                  static char* res="int";
                  return res;               
               }             
               break;
                       
      default:
               {
                  static char* res="orm_unknown_db_type_name";
                  return res;
               
               }               
   
   }//end of switch
   
}//end of function



inline
std::string orm_fld_typecast (const int ftype, const int len)
{
   //typecast on postgres is
   // fldname::<type>
   //or
   // :phXyz<bigint>::bigint
   /* example:
   otl_stream
   o(10, // buffer size
     "UPDATE test_tab SET f2= t.f2,f3=t.f3 from"
     "(VALUES"
     "( :f1<bigint>\\:\\:bigint, :f2<char[31]>, :f3<bigint>\\:\\:bigint)"
     ")"
     "as t(f1,f2,f3) WHERE test_tab.f1=t.f1",
     db 
    );
      
   */
   std::string res=std::string("\\:\\:")+std::string(orm_db_type_name(ftype,len));
   return res;

}

//this is how we must name bound variables 
inline const std::string orm_var_type_name(const int ftype, const int len)
{
   //my orm var types are starting with 1000 to avoid conflicts
   //with otl var type names
   if (ftype>1000)
   {

#ifdef OTL_ODBC_POSTGRESQL   
      //const char* const_DECIMAL="\\:\\:varchar";   
      const char* const_DECIMAL="CHAR[43]";   
      //CMONEY_STR_MAXLEN is 43
      
      //see OTL example 676
      const char* const_TIMEZONE="CHAR[60]";
#else
   #error define the OTL acceptable field type for DECIMAL and TIMEZONE
#endif      
      switch (ftype)
      {
         case orm_var_decimal: return const_DECIMAL;
         case orm_var_timestamp: return const_TIMEZONE;
         case orm_var_timestamp_tz: return const_TIMEZONE;
         default: return "UNKNOWN FTYPE";
      }         
   }
   else
   {
    //  return otl_var_type_name(ftype);

      std::string resbuf;
              
      std::string datatype;
      switch (ftype)
      {
case otl_var_none: assert(0);return std::string("UNKNOWN FTYPE");break;
case otl_var_char:
   {      
      resbuf=std::string("CHAR")+"["+boost::lexical_cast<std::string>(len)+"]";
   }   
   break;
case otl_var_double:
   resbuf="DOUBLE";
   break;
case otl_var_float:
   resbuf="FLOAT";
   break;
case otl_var_int:
   resbuf="INT";
   break;
   
case otl_var_unsigned_int:
   resbuf="UNSIGNED";
   break;

case otl_var_short:
   resbuf="SHORT";
   break;

case otl_var_long_int:
   resbuf="LONG";
   break;
case otl_var_timestamp:
   resbuf="TIMESTAMP";
   break;
   
case otl_var_varchar_long:   
   resbuf="VARCHAR_LONG";
   break;
case otl_var_raw_long:
   resbuf="raw_long";
   break;
case otl_var_clob:
   resbuf="clob";
   break;
case otl_var_blob:
   resbuf="blob";
   break;
case otl_var_refcur:      
   assert(0);//need to support length of cursor   
   resbuf="REFCUR";
   break;
case otl_var_long_string:
   resbuf="varchar_long";
   break;

case otl_var_db2time:
   resbuf="db2time";   
   break;
case otl_var_db2date:
   resbuf="db2time";
   break;

case otl_var_tz_timestamp:
   resbuf="TZ_TIMESTAMP";
   break;
case otl_var_ltz_timestamp:
      resbuf="LTZ_TIMESTAMP";
      break;
case otl_var_bigint:
   resbuf="BIGINT";
   break;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
case otl_var_nchar:
   resbuf="NCHAR";
   break;
case otl_var_nclob=22;
   resbuf="NCLOB";
   break;
#else
#endif
case otl_var_raw:
   {
      resbuf="raw["+boost::lexical_cast<std::string>(len)+"]";      
   }
   break;      
case otl_var_lob_stream:
   assert(0);
   resbuf="otl_var_lob_stream";
   break;
default:
   assert(0);    
      
      }//end of switch
   return resbuf;      
   }//end of else
   
}//end of function





inline
std::ostream& operator <<(std::ostream& o,const otl_var_desc& d)
{
o<<"param_type: " <<d.param_type<<std::endl;
o<<"ftype: "<<d.ftype<<std::endl; 	
o<<"elem_size: "<<d.elem_size<<std::endl;
o<<"array_size: "<<d.array_size<<std::endl;
o<<"pos: "<<d.pos<<std::endl;
o<<"name_pos: "<<d.name_pos<<std::endl;
o<<"name[128]: "<<d.name<<std::endl;
o<<"pl_tab_flag: "<<d.pl_tab_flag<<std::endl;
return o;
}


///if you have a select call this function
//in debug mode to see how OTL understands
//the output variables of the select 
inline
void log__otl_stream_outvars (otl_stream& s)
{
   int len;
   otl_var_desc* darr;
   darr=s.describe_out_vars(len);
   for (int x=0;x<len;x++)
   {
      std::cout<<darr[x]<<std::endl;
   }   
}



//
// end of functions that augment otl
////////////////////////////////////////////




      ///Prepared SQL stored with each connection. Relies on unique tags
struct psqlconn: public boost::noncopyable,
                    boost::enable_shared_from_this<psqlconn>
{
      
   typedef boost::shared_ptr<otl_connect> tConn_ptr;
   typedef boost::shared_ptr< otl_stream> tpSQL_ptr;
      
   typedef std::map< std::string, boost::shared_ptr< otl_stream> > tPSQLs;
   typedef std::map< std::string, boost::shared_ptr< otl_stream> >::const_iterator tPSQLs_const_it;
   typedef std::map< std::string, boost::shared_ptr< otl_stream> >::iterator tPSQLs_it;
      
      
   psqlconn (tConn_ptr conn_ptr,const std::string& indbnm="")
   :c(conn_ptr),m_target_schema("prod"),
   m_dbnm(indbnm)/*helps debugging what db we are dealing with*/
   {
         c->set_max_long_size(32767*2);   
         c->auto_commit_off();
         long res;
         
          res=  otl_cursor::direct_exec
             (
               *c,
                  "SET client_encoding='UTF8';",
                  otl_exception::disabled // disable OTL exceptions
               ); 

          
          std::string set_schema=
#if defined(OTL_ODBC_POSTGRESQL)          
          "set search_path to "+m_target_schema+";";
#else
      #error define for your db how to set default schema
#endif                
          
          res=  otl_cursor::direct_exec
             (
               *c,
                  set_schema.c_str(),
                  otl_exception::disabled // disable OTL exceptions
               ); 


          res=  otl_cursor::direct_exec
             (
               *c,
                  "commit;",
                  otl_exception::disabled // disable OTL exceptions
               ); 

               //begin transaction (otherwise PG autommits everything
               //let's not begin 
               //begin();               
   }
   
   virtual ~psqlconn (void)
   {
      //by default OTL commits the work on logoff
      //I certainly do not want that behaviour
      //
      
      this->rollback();
      c->logoff();   
   }
   

   //typecast
   operator otl_connect& ()
   {
      return *c;
   }      
   tConn_ptr connection (void)
   {
      return c;
   }
   
   void rollback (void)
   {
   
      c->rollback();
   }
   
   void commit (void)
   {
      c->commit();
   }
   
   void begin (void)
   {
//notify the developer that most likely he does not need this
//function (or she)   
clog<<"psqlconn::begin is not portable and typically is not needed"<<endl;     
      otl_cursor::direct_exec
      (
         *c,
         "START TRANSACTION;",
         otl_exception::disabled // disable OTL exceptions
      ); 
   
   }
   
         
   ///add a new sql if does not exist
   //returns an existing or a new stream ptr         
   tpSQL_ptr add_ifnotthere (const std::string& tag,
                        const std::string& sqlstmt,
                        
                        bool& isnew, //tell the caller if this was new,
                        const int implicit_select=otl_explicit_select,
                        const size_t bulksize=11                       
                        )
   {
      tPSQLs_it it;
      it=pstatements.find(tag);
      if (it==pstatements.end())
      {
         isnew=true;
         
         std::string onexception_str=std::string("[")+m_dbnm+std::string("]")+
            std::string("sqltag: ")+tag;         
         tpSQL_ptr p(new  otl_stream(bulksize,
                        sqlstmt.c_str(),
                        *c,
                        implicit_select,
                        onexception_str.c_str()));
         p->set_commit(0);                        
         //do not flush on destructor
         p->set_flush(0);                        
         pstatements.insert(make_pair(tag,p));    
         return p;                                                              
      }
      else
      {
         isnew=false;
         
         //check if it is actually good, if not create a new one
         if (it->second->good()==false)
         {
            //remove it and create a new one
            //cout<<"existing stream is no good "<<endl;

            destroy_stream (it->second);
            pstatements.erase(it);
            
            //duplicate code, organize better
            isnew=true;
            std::string onexception_str=std::string("[")+m_dbnm+std::string("]")+
            std::string("sqltag: ")+tag;
            tpSQL_ptr p(new  otl_stream(bulksize,
                        sqlstmt.c_str(),
                        *c,
                        implicit_select,
                        onexception_str.c_str()));
                        
            /*  from OTL docs If the stream returns a 
                  result set via a stored procedure call, (VP: or NEXTVAL as example)
                  this parameter needs to be set to otl_implicit_select
            */                        
                        
            p->set_commit(0);                        
            //do not flush on destructor
            p->set_flush(0);                        
            pstatements.insert(make_pair(tag,p));    
            return p;                                                
            
            //end of duplicate code
         }
         else
         {
            return it->second;
         }
      }   
   }                        

   //will not say anything if the stream is not here
   void remove (const std::string& tag)   
   {
      tPSQLs_it it;
      it=pstatements.find(tag);
      if (it!=pstatements.end())
      {
         destroy_stream (it->second);
         pstatements.erase(it);
      }   
   }
   void add (const std::string& tag, 
                  tpSQL_ptr stmt)
   {
      pstatements.insert(make_pair(tag,stmt));
   }
   
   
   void destroy_stream (tpSQL_ptr ptr)
   {
      ptr->clean(1);
      ptr->close();
   }
   
   //reserve next id 
   //this 'commits' the next id, so if you call it repeatedly
   //the sequence value will be incremented
   template <typename TRes>
   void reserve_seq_id (const std::string& tbnm,
                        const std::string& fldnm,
                        TRes& nextid)
   {                           
   
   #if defined (OTL_ODBC_POSTGRESQL)
std::string seqnm;
seqnm=schema() + 
            std::string(".") + 
            tbnm + 
            std::string("_") +
            fldnm + 
            std::string("_seq");
            

std::string sql=
 std::string("SELECT NEXTVAL('")+seqnm+std::string("')");

//a short cut that will insure that we have unique
//streams for each sequence 
std::string sql_tag=sql;


bool isnew;
psqlconn::tpSQL_ptr s = 
            this->add_ifnotthere(sql_tag, //unique tag for the sql
                                sql, //sql itself
                                isnew,
                                otl_implicit_select,/*this is critical when result is returned via function */
                                1/*bulk size*/);  //isnew flag will be true if new tag 

/*

	Rewind the stream. If the stream does not have any input 
	variables, this function forces the stream to execute 
	its SQL statement.
*/
	s->rewind();


 //  *s<<to_csqlstring(seqnm);//this will also do an execute             
   while(!s->eof()){ // while not end-of-data
   *s>>nextid;  
   }             



#else
   #error define get last ID for your database
#endif      

   }
   
    const std::string& schema(void) const
    {
      return m_target_schema;
    }     
         
/*         
   tpSQL_ptr statement(const std::string& tag,
                               bool& isnew)
   {
      tPSQLs_it it;
      it=pstatements.find(tag);
      if (it!=pstatements.end())
      {
         isnew=false;
          return (it->second);
      }
      else
      {
         isnew=true;
         tpSQL_ptr p(new otl_stream());
         //notice I do not add the new ones to the internal pool
               //I expect that add will be called
           return p;                           
      }
     }
*/     
         
      tConn_ptr c;
      //list of streams per connection;
      tPSQLs pstatements;      
      std::string m_target_schema;
      std::string m_dbnm;/*helps debuggin to know what db we are dealing with */
   };
   
   




struct tField
{
   private:
      int order;
      int otl_fld_typ;
      int field_length;
      bool nullable;
      std::string name;   
      //dummy constructor only can be invoked 
      //within this class
      tField (void) {};
      
   public:      
   
   tField (
            const int in_order,
            const int in_otl_fld_typ,
            const int in_field_length,
            const bool in_nullable,
            const std::string& in_name)
   :order(in_order)
   ,otl_fld_typ(in_otl_fld_typ)
   ,field_length(in_field_length)
   ,nullable(in_nullable)
   ,name (in_name)
            
  {}            
   
  tField (const tField& in)
  {
      *this=in;
  } 
   
   tField& operator=(const tField &in)
   {
      order=in.order;
      otl_fld_typ=in.otl_fld_typ;
      field_length=in.field_length;
      nullable=in.nullable;
      name= in.name;
      return *this;
   }

   
   
   const std::string& fname(void) const
   {
      return name;
   }
   
   //field name for SQL out parameters
   const std::string sqlout_fname(void) const
   {
      switch (fotltyp())
      {
      case orm_var_decimal:
      case orm_var_timestamp:
      case orm_var_timestamp_tz:
      {
#ifdef OTL_ODBC_POSTGRESQL      
         const char* postgresql_typecast="\\:\\:varchar";         
         return fname()+postgresql_typecast;
#else
   #error  define how selects should be treating the field types above
#endif         
      }
      
      default:
         return fname();
      }      
   }


   //for inserts when data is sent into the database
   const std::string sqlin_fname(void) const
   {
      switch (fotltyp())
      {
      case orm_var_decimal:
#ifdef OTL_ODBC_POSTGRESQL      
      {
            const char* postgresql_typecast="\\:\\:DECIMAL(19,6)";         
            return fname()+postgresql_typecast;
      }         
#else
   #error define if ther eis a typecast for the decimal field for your db
#endif            
      case orm_var_timestamp:
#ifdef OTL_ODBC_POSTGRESQL            
         {         
            const char* postgresql_typecast="\\:\\:TIMESTAMP";         
            return fname()+postgresql_typecast;
         }   
#else
   #error  define how selects should be treating the field types above
#endif         
         
      
      case orm_var_timestamp_tz:
      {
#ifdef OTL_ODBC_POSTGRESQL      
         {
            const char* postgresql_typecast="\\:\\:TIMESTAMP with time zone";         
            return fname()+postgresql_typecast;
         }   
#else
   #error  define how selects should be treating the field types above
#endif         
      }
      
      default:
         return fname();
      }      
   }

   const std::string ftype_dbname (void) const
   {
      std::string resbuf;
      switch (fotltyp())
      {
      case orm_var_decimal:resbuf="DECIMAL (19,6)";break;
      case orm_var_timestamp:resbuf="TIMESTAMP";break;
      case orm_var_timestamp_tz:resbuf="TIMESTAMP WITH TIME ZONE";break;
      
      
      case otl_var_none: assert(0);return std::string("UNKNOWN FTYPE");break;
case otl_var_char:
   {      
      resbuf=std::string("CHAR")+"("+boost::lexical_cast<std::string>(flen())+")";
   }   
   break;
case otl_var_double:
   resbuf="DOUBLE";
   break;
case otl_var_float:
   resbuf="FLOAT";
   break;
case otl_var_int:;
   resbuf="INT";
   break;
   
case otl_var_unsigned_int:
   resbuf="UNSIGNED";
   break;

case otl_var_short:
   resbuf="SHORT";
   break;

case otl_var_long_int:
   resbuf="LONG";
   break;
case otl_var_timestamp:
   resbuf="TIMESTAMP";
   break;
   
case otl_var_varchar_long:   
   resbuf="VARCHAR_LONG";
   break;
case otl_var_raw_long:
   resbuf="raw_long";
   break;
case otl_var_clob:
   resbuf="clob";
   break;
case otl_var_blob:
   resbuf="blob";
   break;
case otl_var_refcur:      
   assert(0);//need to support length of cursor   
   resbuf="REFCUR";
   break;
case otl_var_long_string:
   resbuf="varchar_long";
   break;

case otl_var_db2time:
   resbuf="db2time";   
   break;
case otl_var_db2date:
   resbuf="db2time";
   break;

case otl_var_tz_timestamp:
   resbuf="TIMESTAMP WITH TIME ZONE";
   break;
case otl_var_ltz_timestamp:
      resbuf="LTZ_TIMESTAMP";
      break;
case otl_var_bigint:
   resbuf="BIGINT";
   break;
#if defined(OTL_ORA_UNICODE)||defined(OTL_ORA_UTF8)
case otl_var_nchar:
   resbuf="NCHAR";
   break;
case otl_var_nclob=22;
   resbuf="NCLOB";
   break;
#else
#endif
case otl_var_raw:
   {
      resbuf="raw("+boost::lexical_cast<std::string>(flen())+")";      
   }
   break;      
case otl_var_lob_stream:
   assert(0);
   resbuf="DO NOT KNOW otl_var_lob_stream";
   break;
default:
   assert(0);    
      
      }//end of switch

      
   
   return resbuf;
   }

   
   
   int forder(void) const
   {
      return order;
   }
   
   int fotltyp(void) const
   {
      return otl_fld_typ;
   }
   int flen (void) const
   {
      return field_length;
   }
   bool fisnullable(void) const
   {
      return nullable;
   }
   
   static tField getdummy(void)
   {
   
      return tField();
   }   
   
   friend bool operator <(const tField& a, const tField& b)
   {
      return (a.order<b.order);
   }
};

//field functor
struct CFFunctorReceiveVal
{
   template<typename TField> 
   void operator() (const TField& fld)
   {  
      typedef const otl_value<typename TField::tValTyp> t_otlvaltype;
   
      #if defined (OTLEXPR1_USE_ANYTYPE_ONLY)       
      fld.assign_any(getvalue(fld.asfield(),
      typeid(typename TField::tValTyp) ));
      #else
      const otl_value<typename TField::tValTyp>& ref(fld);
      receive_value(fld.asfield(),
                  typeid(typename TField::tValTyp),
                  ref);
                  //(t_otlvaltype&)fld);
      #endif                  
   }
   virtual
   void receive_anyvalue(const tField& attrs,
   		const std::type_info& tinfo, 		
		const boost::any& in)=0;
   

   
   virtual
   void receive_value(const tField& attrs, 
         const std::type_info& tinfo, 
         const otl_value<std::string>& in)
   =0;

#ifdef OTL_UNICODE   
   virtual
   void receive_value(const tField& attrs,
       const std::type_info& tinfo, 
       const otl_value<std::wstring>& in)
   =0;
#endif   
   

#ifdef ORM_HAS_CMONEY
   virtual
   void receive_value(const tField& attrs,
       const std::type_info& tinfo, 
       const otl_value<cmoney_t>& in)
   =0;
#endif   




   virtual
   void receive_value(const tField& attrs,
       const std::type_info& tinfo, 
       const otl_value<float>& in)
   =0;

   
   virtual
   void receive_value(const tField& attrs, const std::type_info& tinfo, const otl_value<double>& in)
   =0;
   
   virtual
   void receive_value(const tField& attrs, const std::type_info& tinfo, const otl_value<OTL_BIGINT>& in)
   =0;
   
   virtual
   void receive_value(const tField& attrs, const std::type_info& tinfo, const otl_value<long>& in)
   =0;

   
   virtual
   void receive_value(const tField& attrs, const std::type_info& tinfo, 
   const otl_value<otl_datetime>& in)
   =0;
};




struct CFFunctorAssignVal 
{   
   template<typename TField> 
   void operator() (TField& fld)
   {

      typedef otl_value<typename TField::tValTyp> t_otlvaltype;
   
      #if defined (OTLEXPR1_USE_ANYTYPE_ONLY)       
      fld.assign_any(getvalue(fld.asfield(),
      typeid(typename TField::tValTyp
      ) ));
      #else
      getvalue(   fld.asfield(),
                  typeid(typename TField::tValTyp),
                  (t_otlvaltype&)fld);
      #endif                  
   }   
   virtual
   boost::any getvalue(const tField& attrs, const std::type_info& tinfo)=0;
   
   
   virtual
   void getvalue(const tField& attrs, 
                  const std::type_info& tinfo, 
                  otl_value<std::string>& in)
   =0;

#ifdef OTL_UNICODE   
   virtual
   void getvalue(const tField& attrs, 
                 const std::type_info& tinfo, 
                 otl_value<std::wstring>& in)
   =0;
#endif   
   
#ifdef ORM_HAS_CMONEY  
   virtual 
   void getvalue(const tField& attrs, 
                 const std::type_info& tinfo, 
                 otl_value<cmoney_t>& in)
   =0;

#endif
   
   
   
   virtual
   void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<float>& in)
   =0;

   virtual
   void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<double>& in)
   =0;
   
   virtual
   void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<OTL_BIGINT>& in)
   =0;
   
   virtual
   void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<long>& in)
   =0;

   
   virtual
   void getvalue(const tField& /*attrs*/, const std::type_info& tinfo, 
   otl_value<otl_datetime>& in)
   =0;
};

/* field level functors start with CF */
struct CFFunctorRead__otlstream: public CFFunctorAssignVal
{
   otl_stream& ref;
   CFFunctorRead__otlstream (otl_stream& in)
      :ref(in)
   {}   
   
   virtual
   boost::any getvalue(const tField& attrs, const std::type_info& tinfo)
   {
      assert(0);
      //can only be used if 
      // OTLEXPR1_USE_ANYTYPE_ONLY
      
      /* based on the type info passed in, guess
         what boost::any_cast<WHATTYPE>  
         to be used (this is at runtime)
         then apply that cast, extract value from
         the any, and then call the appropriate
         methods below
      */
      boost::any ret;
      return ret;
   }
   
   virtual
   void getvalue(const tField& attrs, const std::type_info& tinfo, otl_value<std::string>& in)
   {        
      ref>>in;            
   }
   
   
#ifdef OTL_UNICODE   
   virtual
   void getvalue(const tField& attrs, 
                  const std::type_info& tinfo, 
                  otl_value<std::wstring>& in)
   {        
      ref>>in;            
   }
#endif

#ifdef ORM_HAS_CMONEY
   virtual
   void getvalue(const tField& attrs, 
                  const std::type_info& tinfo, 
                  otl_value<cmoney_t>& in)
   {        
      ref>>in;            
   }
      
#endif
   
   
   
   virtual
   void getvalue(const tField& /*attrs*/, 
		 const std::type_info& /*tinfo*/, otl_value<float>& in)
   {  
      ref>>in;
   }
   
   virtual
   void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<double>& in)
   {  
      ref>>in;
   }
   
   virtual
   void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<OTL_BIGINT>& in)
   {  
      ref>>in;
   }
   
      virtual
      void getvalue(const tField& /*attrs*/, 
		    const std::type_info& /*tinfo*/, otl_value<long>& in)
   {  
      ref>>in;
   }

   
   
   virtual
   void getvalue(const tField& /*attrs*/, const std::type_info& tinfo, 
   otl_value<otl_datetime>& in)
   {  
      //should I use endr ?
      ref>>in;
   }

};

/* receive value means as relates to the Functor's data members .
   Meaning that they are 'receiving'
  Therefore Receive means to write something to the functor
  that means that the values we are passing to it
  can be const (because we are not modifying them...
  */
  /* field level functors start with CF */
struct CFFunctorWrite__otlstream: public CFFunctorReceiveVal
{
   otl_stream& ref;
   CFFunctorWrite__otlstream (otl_stream& in)
      :ref(in)
   {}   
   
   virtual
   void receive_anyvalue(const tField& attrs, 
                        const std::type_info& tinfo, 
                        const boost::any& in)
   {
      assert(0);
      //can only be used if 
      // OTLEXPR1_USE_ANYTYPE_ONLY
      
      /* based on the type info passed in, guess
         what boost::any_cast<WHATTYPE>  
         to be used this is at runtime
         then apply that cast, extract value from
         the any, and then call the appropriate
         methods below
      */

   }
   
   virtual
   void receive_value(const tField& /*attrs*/, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<std::string>& in)
   {  
      ref<<in;
   }
   
#ifdef OTL_UNICODE      
   virtual
   void receive_value(const tField& /*attrs*/, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<std::wstring>& in)
   {  
      ref<<in;
   }
#endif   


#ifdef ORM_HAS_CMONEY
   virtual
   void receive_value(const tField& /*attrs*/, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<cmoney_t>& in)
   {  
      ref<<in;
   }
#endif
   
   virtual
   void receive_value(const tField& /*attrs*/, const std:: type_info& /*tinfo*/, const otl_value<float>& in)
   {  
      ref<<in;
   }
   
   virtual
   void receive_value(const tField& /*attrs*/, const std::type_info& /*tinfo*/, const otl_value<double>& in)
   {  
      ref<<in;
   }
   
   virtual
   void receive_value(const tField& /*attrs*/, const std::type_info& /*tinfo*/, const otl_value<OTL_BIGINT>& in)
   {  
      ref<<in;
   }
   
   virtual
   void receive_value(const tField& /*attrs*/, const std::type_info& /*tinfo*/, const otl_value<long>& in)
   {  
      ref<<in;
   }
   
   
   virtual
   void receive_value(const tField& /*attrs*/, const std::type_info& tinfo, 
   const otl_value<otl_datetime>& in)
   {  
      ref<<in;
   }

};

//field level functors start with CF
struct CFFunctorWrite__ostream: public CFFunctorReceiveVal
{
   std::ostream& ref;
   std::string field_delim_start;
   std::string attr_delim;
   bool pretty;
   CFFunctorWrite__ostream (std::ostream& in,
   const char* in_field_delim_start="\t|",
   const char* in_attr_delim=" - ",
   const bool in_pretty=true
   )      
      :ref(in),
      field_delim_start (in_field_delim_start),
      attr_delim (in_attr_delim),
      pretty(in_pretty)
   {}   
   
   virtual ~CFFunctorWrite__ostream(){}
   
   virtual
   void receive_anyvalue(const tField& attrs, 
                        const std::type_info& tinfo, 
                        const boost::any& in)
   {
      assert(0);
      //can only be used if 
      // OTLEXPR1_USE_ANYTYPE_ONLY
      
      /* based on the type info passed in, guess
         what boost::any_cast<WHATTYPE>  
         to be used this is at runtime
         then apply that cast, extract value from
         the any, and then call the appropriate
         methods below
      */
         
   }
   
   virtual
   void receive_value(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<std::string>& in)
   {  
      ref<<"["<<in<<"]"<<" defined as: ";
      showattr(attrs);
      ref<<std::endl;
   }

#ifdef OTL_UNICODE      
   virtual
   void receive_value(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<std::wstring>& in)
   {  
      ref<<"["<<in<<"]"<<" defined as: ";
      showattr(attrs);
      ref<<std::endl;
   }

#endif   



#ifdef ORM_HAS_CMONEY
   virtual
   void receive_value(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<cmoney_t>& in)
   {  
      ref<<"["<<in<<"]"<<" defined as: ";
      showattr(attrs);
      ref<<std::endl;
   }

#endif   


   
   
   virtual
   void receive_value(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<float>& in)
   {  
      ref<<"["<<in<<"]"<<" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }
   
   virtual
   void receive_value(const tField& attrs, const std::type_info& /*tinfo*/, const otl_value<double>& in)
   {  
      ref<<"["<<in<<"]"<<" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }
   
   virtual
   void receive_value(const tField& attrs, const std::type_info& /*tinfo*/, const otl_value<OTL_BIGINT>& in)
   {  
      ref<<"["<<in<<"]"<<" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }


   virtual
   void receive_value(const tField& attrs, const std::type_info& /*tinfo*/, 
                  const otl_value<long>& in)
   {  
      ref<<"["<<in<<"]"<<" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }


   
   virtual
   void receive_value(const tField& attrs, const std::type_info& tinfo, 
   const otl_value<otl_datetime>& in)
   {  
      ref<<"["<<in<<"]"<<" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }
   
   virtual
   void showattr (const tField& attrs)
   {        
      ref<<attrs.fname();
      ref<<" "<<orm_var_type_name(attrs.fotltyp(),attrs.flen());      
      if (attrs.flen()>0)
      {
         ref<<" ("<<attrs.flen()<<")";
      }
      if (attrs.fisnullable()==false)
      {
         ref<<" NOT NULL ";
      }
      else
      {
         ref<<" /* NULLABLE */ ";
      }      
      ref<<" /* ORDER: "<<attrs.forder()<<" */";
   }

};


//Wostream version
struct CFFunctorWrite__wostream: public CFFunctorReceiveVal
{
   std::wostream& ref;
   std::string field_delim_start;
   std::string attr_delim;
   bool pretty;
   CFFunctorWrite__wostream (std::wostream& in,
   const char* in_field_delim_start="\t|",
   const char* in_attr_delim=" - ",
   const bool in_pretty=true
   )      
      :ref(in),
      field_delim_start (in_field_delim_start),
      attr_delim (in_attr_delim),
      pretty(in_pretty)
   {}   
   
   virtual ~CFFunctorWrite__wostream(){}
   
   virtual
   void receive_anyvalue(const tField& attrs, 
                        const std::type_info& tinfo, 
                        const boost::any& in)
   {
      assert(0);
      //can only be used if 
      // OTLEXPR1_USE_ANYTYPE_ONLY
      
      /* based on the type info passed in, guess
         what boost::any_cast<WHATTYPE>  
         to be used this is at runtime
         then apply that cast, extract value from
         the any, and then call the appropriate
         methods below
      */
         
   }
   
   virtual
   void receive_value(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<std::string>& in)
   {  
      //since the stream is double byte, we must convert
      //the string into the double byte
      
      std::wstring tmp=UtfConverter::FromUtf8(in.v) ;
   
      ref<<L"["<<tmp<<L"]"<<L" defined as: ";
      showattr(attrs);
      ref<<std::endl;
   }
   
#ifdef OTL_UNICODE      
   virtual
   void receive_value(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<std::wstring>& in)
   {  
      
      ref<<L"["<<in.v<<L"]"<<L" defined as: ";
      showattr(attrs);
      ref<<std::endl;
   }
#endif
   




#ifdef ORM_HAS_CMONEY
   virtual
   void receive_value(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<cmoney_t>& in)
   {  
      
      ref <<L"[" <<in.v<<L"]" <<L" defined as: ";
      showattr(attrs);
      ref<<std::endl;
   }
#endif



   
   
   virtual
   void receive_value(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     const otl_value<float>& in)
   {  
      ref<<L"["<<in<<L"]"<<L" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }
   
   virtual
   void receive_value(const tField& attrs, const std::type_info& /*tinfo*/, const otl_value<double>& in)
   {  
      ref<<L"["<<in<<L"]"<<L" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }
   
   virtual
   void receive_value(const tField& attrs, const std::type_info& /*tinfo*/, const otl_value<OTL_BIGINT>& in)
   {  
      ref<<L"["<<in<<L"]"<<L" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }


   virtual
   void receive_value(const tField& attrs, const std::type_info& /*tinfo*/, 
                  const otl_value<long>& in)
   {  
      ref<<L"["<<in<<L"]"<<L" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }


   
   virtual
   void receive_value(const tField& attrs, const std::type_info& tinfo, 
   const otl_value<otl_datetime>& in)
   {  
      ref<<L"["<<in<<L"]"<<L" defined as: ";
      showattr(attrs);
      ref<<std::endl;

   }
   
   virtual
   void showattr (const tField& attrs)
   { 
      std::wstring tmp=UtfConverter::FromUtf8(attrs.fname()) ;       
      ref<<tmp;
      ref<<L" "<<UtfConverter::FromUtf8(orm_var_type_name(attrs.fotltyp(),attrs.flen()));      
      if (attrs.flen()>0)
      {
         ref<<L" ("<<attrs.flen()<<L")";
      }
      if (attrs.fisnullable()==false)
      {
         ref<<L" NOT NULL ";
      }
      else
      {
         ref<<L" /* NULLABLE */ ";
      }      
      ref<<L" /* ORDER: "<<attrs.forder()<<L" */";
   }

};


struct CFFunctorSet__to_randomval: public CFFunctorAssignVal
{
   //somestream ref;
   CFFunctorSet__to_randomval (void)
   {}   
   
   virtual
     boost::any getvalue(const tField& attrs, const std::type_info& tinfo)
   {
      assert(0);
      //can only be used if 
      // OTLEXPR1_USE_ANYTYPE_ONLY
      
      /* based on the type info passed in, guess
         what boost::any_cast<WHATTYPE>  
         to be used (this is at runtime)
         then apply that cast, extract value from
         the any, and then call the appropriate
         methods below
      */
      boost::any res;
      return res;

   }
   
   virtual
     void getvalue(const tField& attrs, const std::type_info& tinfo, otl_value<std::string>& in)
   {  
      assign_random_val(in);
      in.set_non_null();
   }
#ifdef OTL_UNICODE      
   virtual
     void getvalue(const tField& attrs, 
     const std::type_info& tinfo, 
     otl_value<std::wstring>& in)
   {  
      assign_random_val(in);
   }
#endif


#ifdef ORM_HAS_CMONEY
   virtual
     void getvalue(const tField& attrs, 
     const std::type_info& tinfo, 
     otl_value<cmoney_t>& in)
   {  
      assign_random_val(in);
      in.set_non_null();
   }
#endif



   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<float>& in)
   {  
      assign_random_val(in);
      in.set_non_null();
   }
   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<double>& in)
   {  
      assign_random_val(in);
      in.set_non_null();
   }
   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<OTL_BIGINT>& in)
   {     
      assign_random_val(in);
      in.set_non_null();
   }
   
   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<long>& in)
   {     
      assign_random_val(in);
      in.set_non_null();
   }

   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& tinfo, 
     otl_value<otl_datetime>& in)
   {  
      assign_random_val(in);
      in.set_non_null();
   }

};






//field level functor
struct CFFunctorSet__to_otlnull: public CFFunctorAssignVal
{

   otl_null ref;
   CFFunctorSet__to_otlnull (void)
   {}   
   
   virtual
     boost::any getvalue(const tField& attrs, const std::type_info& tinfo)
   {
      assert(0);
      //can only be used if 
      // OTLEXPR1_USE_ANYTYPE_ONLY
      
      /* based on the type info passed in, guess
         what boost::any_cast<WHATTYPE>  
         to be used (this is at runtime)
         then apply that cast, extract value from
         the any, and then call the appropriate
         methods below
      */
      boost::any res;
      return res;

   }
   
   virtual
     void getvalue(const tField& attrs, const std::type_info& tinfo, otl_value<std::string>& in)
   {  
      in=ref;
   }
#ifdef OTL_UNICODE      
   virtual
     void getvalue(const tField& attrs, 
     const std::type_info& tinfo, 
     otl_value<std::wstring>& in)
   {  
      in=ref;
   }
#endif


#ifdef ORM_HAS_CMONEY
   virtual
     void getvalue(const tField& attrs, 
     const std::type_info& tinfo, 
     otl_value<cmoney_t>& in)
   {  
      in=ref;
   }
#endif



   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<float>& in)
   {  
      in=ref;
   }
   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<double>& in)
   {  
      in=ref;
   }
   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<OTL_BIGINT>& in)
   {     
      in=ref;
   }
   
   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& /*tinfo*/, otl_value<long>& in)
   {     
      in=ref;
   }

   
   virtual
     void getvalue(const tField& /*attrs*/, const std::type_info& tinfo, 
     otl_value<otl_datetime>& in)
   {  
      in=ref;
   }
};



#ifdef USE_BOOST_SERIALIZE
//This functor is for serilazing row into file
//using Boost serialize




//field level functors start with CF
/* serialize From archive, in the functions
   here the field references will get values assigned to them

   For serialize this is the only functor  because it does both
   /save and load /  so if you want to rows/tables to be serialized
   they cannot be 'const'
*/
template <typename Archive>
struct CFFunctorSerializeFrom__boost: public CFFunctorAssignVal
{
   Archive& ref; 
   CFFunctorSerializeFrom__boost (Archive& ar,const unsigned int version=0)      
      :ref(ar)
   {}   
   
   virtual ~CFFunctorSerializeFrom__boost(){}
   

   virtual
   boost::any getvalue(const tField& attrs, const std::type_info& tinfo)
   {
      assert(0);
      //can only be used if 
      // OTLEXPR1_USE_ANYTYPE_ONLY
      
      /* based on the type info passed in, guess
         what boost::any_cast<WHATTYPE>  
         to be used this is at runtime
         then apply that cast, extract value from
         the any, and then call the appropriate
         methods below
      */

        boost::any ret;
      return ret;
         
   }
   
   virtual
   void getvalue(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     otl_value<std::string>& in)
   {  
     ref &boost::serialization:: make_nvp(attrs.fname().c_str(), in.v);
     if (Archive::is_loading::value)
     {
       //set to not null only when loading
      in.set_non_null();
     }
   }

#ifdef OTL_UNICODE      
   virtual
   void getvalue(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     otl_value<std::wstring>& in)
   { 
     ref & boost::serialization::make_nvp(attrs.fname().c_str(), in.v);

     if (Archive::is_loading::value)
     {
      in.set_non_null();
     }
   }

#endif   



#ifdef ORM_HAS_CMONEY
   virtual
   void getvalue(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     otl_value<cmoney_t>& in)
   {  
     if (Archive::is_loading::value)
     {
        std::string tmp;
        ref & boost::serialization::make_nvp(attrs.fname().c_str(), tmp);
        in.v.from_str(tmp);
        in.set_non_null();
     }
     else
     {
#ifdef OTL_UNICODE
        std::wstring tmp(in.v.as_wstr());
#else
        std::string tmp(in.v.as_str());
#endif
        ref & boost::serialization::make_nvp(attrs.fname().c_str(), tmp);
     }

   }

#endif   


   
   
   virtual
   void getvalue(const tField& attrs, 
                     const std::type_info& /*tinfo*/, 
                     otl_value<float>& in)
   {  
    ref & boost::serialization::make_nvp(attrs.fname().c_str(), in.v);
    if (Archive::is_loading::value)
    {
      in.set_non_null();
    }
   }
   
   virtual
   void getvalue(const tField& attrs, const std::type_info& /*tinfo*/, 
                 otl_value<double>& in)
   {  
      ref & boost::serialization::make_nvp(attrs.fname().c_str(), in.v);
      if (Archive::is_loading::value)
      {
        in.set_non_null();
      }
   }
   
   virtual
   void getvalue(const tField& attrs, const std::type_info& /*tinfo*/,
                  otl_value<OTL_BIGINT>& in)
   {  
      ref & boost::serialization::make_nvp(attrs.fname().c_str(), in.v);
      if (Archive::is_loading::value)
      {
        in.set_non_null();
      }
   }


   virtual
   void getvalue(const tField& attrs, const std::type_info& /*tinfo*/,
                  otl_value<long>& in)
   {  
      ref & boost::serialization::make_nvp(attrs.fname().c_str(), in.v);
      if (Archive::is_loading::value)
      {
        in.set_non_null();
      }
   }

   
   virtual
   void getvalue(const tField& attrs, const std::type_info& tinfo, 
                  otl_value<otl_datetime>& in)
   {
    if (Archive::is_loading::value)
    {
//#ifdef OTL_UNICODE
  //    std::wstring tmp;
//#else
      std::string tmp;
//#endif
      //read the value from the archive
      ref & boost::serialization::make_nvp(attrs.fname().c_str(), tmp);
      //now assign it to the in.v
     
      OTL_ODBC_STRING_TO_TIMESTAMP(tmp.c_str(),in.v)
      in.set_non_null();
    }
    else
    {
      //OTL_ODBC_TIMESTAMP_TO_STRING(tm,str) 

      char buf[256];
      OTL_ODBC_TIMESTAMP_TO_STRING(in.v,buf) 
      
      ref & boost::serialization::make_nvp(attrs.fname().c_str(),
      std::string(buf));
    }

   }//end of function
   

};//end of CFFunctorSerializeFrom__boost







#endif //USE_BOOST_SERIALIZE










// --- END OF FUNCTORS ----















///Inherit from this class when
//declaring your rows
struct cactiverow_t: public boost::enable_shared_from_this<cactiverow_t>
{
   //it is here just because I need a default
   static boost::mutex do_not_use_mutex;
   static long long do_not_use_ll;

   typedef std::map<std::string, tField> tName2Fields;
   typedef std::map<std::string, tField>::iterator tName2FieldsIter;   
   typedef std::map<std::string, tField>::const_iterator tName2FieldsConstIter; 
   
   typedef std::set<tField> tOrderedFields;
   typedef std::set<tField>::iterator tOrderedFieldsIter;
   typedef std::set<tField>::const_iterator tOrderedFieldsConstIter;
   
   
   //define functions that will populate 'res' as
   //a boost::any field 
   /*
   typedef boost::function<void(cactiverow_t& ,boost::any& res_v )> tFget__field ;
   typedef boost::function<const boost::any (const cactiverow_t&)> tFgetConst__field ;
   */
   
   //assign a value to the field. The value is obtained from functor
   //that's why we call it 'set'
   typedef void (*tFset__field)(cactiverow_t& , CFFunctorAssignVal&)  ;
   
   //send the field value (const) to somewhere
   //that's why we call it 'get'
   typedef void (*tFget__field)(const cactiverow_t&,CFFunctorReceiveVal&)  ;
   
   
   //now define a map that will store the field name and the
   //function pointer
   //note, I am not using references to functors
   //because I think I do not need them (because these
   //functors do not store any state or access any global
   //data
   typedef std::map<std::string,tFset__field> tMFset__field;
   typedef std::map<std::string,tFset__field>::iterator tMFset__fieldIter;
   typedef std::map<std::string,tFset__field>::const_iterator tMFset__fieldConstIter;
   
   typedef std::map<std::string,tFget__field> tMFget__field;
   typedef std::map<std::string,tFget__field>::iterator tMFget__fieldIter;
   typedef std::map<std::string,tFget__field>::const_iterator tMFget__fieldConstIter;
  
  
   //set and get bits in the initialization flag
   static void init_flag_set_ok_Fields (long long& flag)
   {
      //16 numbers after x because each number is just 4 bits
      //and we need 64
      flag=flag | 0x0000000000000001;
   }
   static void init_flag_set_ok_Getters (long long& flag)
   {
      flag=flag | 0x0000000000000002;
   }

   static void init_flag_set_ok_Setters (long long& flag)
   {
      flag=flag | 0x0000000000000004;
   }
  


   static bool init_flag_is_ok_Fields (const long long& flag)
   {
      if (flag & 0x0000000000000001)
         return true; 
      else       
       return false;
   
   }
   static bool init_flag_is_ok_Getters (const long long& flag)
   {
      if (flag & 0x0000000000000002)
       return true; 
      else       
       return false;
   }

   static bool init_flag_is_ok_Setters (const long long& flag)
   {
      if(flag & 0x0000000000000004)
         return true; 
      else       
         return false;
   }




  

    virtual const tName2Fields& get_map_fields (void) const =0;
    /*
    {
         static tName2Fields dummy;
         return dummy;
    }*/

   virtual boost::mutex& get_activerow_static_mutex (void) const =0;
   virtual long long& get_activerow_static_initflag (void) const =0;
  
  
   /*get the map representing the 'getters' of the field */
   virtual tMFget__field& get_map_getfunctors (void)
   {
         static tMFget__field dummy;
         return dummy;
   }
   
   virtual const tMFget__field& get_map_getfunctors (void)const
   {
     static tMFget__field dummy;
     return dummy;
   }

   /*get the map that stores the 'setters' as relates to field */
   virtual tMFset__field& get_map_setfunctors (void)
   {
      static tMFset__field dummy;
      return dummy;
   }

   virtual const tMFset__field& get_map_setfunctors (void) const
   {
      static tMFset__field dummy;
      return dummy;
   }

  
  
   static void register_field_setter_functor (const char* field_nm,
                                             const tFset__field fn,
                                             tMFset__field& map,
                                             boost::mutex&,
                                             long long& initflag); 
  
  static void register_field_getter_functor (const char* field_nm,
                                             tFget__field fn,
                                             tMFget__field& map,
                                             boost::mutex&,
                                             long long& initflag);  
   
   
   static void register_field (const char* field_nm,
                                         tName2Fields&,
                                       const int order=0,
                                       const int otl_fld_typ=0,
                                       const int field_length=0,
                                       const bool nullable=true,
                                        boost::mutex& mtx=do_not_use_mutex,
                                        long long& initflag=do_not_use_ll
                                        );
                 
                                        
   static tOrderedFields get_ordered_fields (const tName2Fields& field_map,const cactiverow_t& r);
   static tOrderedFields get_some_ordered_fields (const tName2Fields& field_map,
                                          const std::vector<std::string>& names);
   static std::string get_bind_str (const tOrderedFields& fld_ordered);
   static std::string get_fieldslist_str (const tOrderedFields& fld_ordered,
   const std::string& prefix="");

   static std::string get_fieldslist_str_sqlout (const tOrderedFields& fld_ordered,
   const std::string& prefix="");

   static std::string get_fieldslist_str_sqlin (const tOrderedFields& fld_ordered,
   const std::string& prefix="");

   template <typename TRow>
   static std::string getscript_createtable (const std::string& tbnm);


   
   static tField asfield(const std::string& fname,
                         const tName2Fields& field_map);
   
      
   virtual void write_values_to_fields(const tOrderedFields& field_names,
                                  cactiverow_t& row,
                                  CFFunctorAssignVal& functor);
  
   
   
   virtual void read_values_from_fields(const tOrderedFields& field_names, 
                                       const cactiverow_t& row,
                                       CFFunctorReceiveVal& functor) const;
                                       
                                       
   //in PG to force the database to use the default value
   //as needed for example in sequence generation
   //I need to specify the word 'DEFAULT'                                       
   template <typename T>                                    
   static std::string  field_default (const T& val)
   {
      return std::string("DEFAULT");
   
   }                                       

   //Row level functions
   //for these functions to work with std algorithms
   //such that the algorithms are applied to the instances
   //of shared_ptr <derived class> this functions
   //cannot be static 
   //also we must use boost::mem_fn and not std::mem_fn
   //because std::mem_fn does not use get_pointer on
   //the template argument to dereference the
   //shared ptr   
   
   /* here are two examples of how to use std algorithms 
       assuming srows is a set of a derived row class
       each derived class has a typedef for the
       set and map of shared ptrs
   */
   /* bind's first argument has to be declared
      because it is implicit this
   */
   /*   
   for_each(srows.begin(),srows.end(),
   boost::bind(&cactiverow_t::assign_otlnull,_1)
   );
   */   

   /*
   for_each(srows.begin(),srows.end(),
   boost::mem_fn(&cactiverow_t::assign_otlnull)
   );
   */
   
    void assign_otlnull (void);
    
    
    /* example of how to use assign_otlnull_somefields */
    /*      
      for_each(srows.begin(),srows.end(),
      boost::bind(&cactiverow_t::assign_otlnull_somefields,_1,fieldnames)
      );
    */
    void assign_otlnull_somefields (const std::vector<std::string>& fields);




    void assign_randomval (void);
    void assign_randomval_somefields (const std::vector<std::string>& fields);

#if defined(USE_BOOST_SERIALIZE)

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);

    /*
    template <typename Archive>
    void serializeFrom__boost(Archive& ar, const unsigned int version);
    */
#endif


#if defined(USE_s11n_SERIALIZE)
    //not working yet just a stub
    template <typename Archive>
    void serializeTo__s11n(Archive& ar, const unsigned int version);

    template <typename Archive>
    void serializeFrom__s11n(Archive& ar, const unsigned int version);
#endif


   


   //from stream to row
   friend otl_stream& operator>>(otl_stream& s, cactiverow_t& row);
   friend otl_stream& operator>>(otl_stream& s, cactiverow_t* row);
   
   //from row to stream
   friend otl_stream& operator<<(otl_stream& s, const cactiverow_t& row);
   friend otl_stream& operator<<(otl_stream& s, const cactiverow_t* row);
   
   //from row to stream
   friend std::ostream& operator<<(std::ostream& s, const cactiverow_t& row);   
   friend std::ostream& operator<<(std::ostream& s, const cactiverow_t* prow);
   

   //from row to stream
   friend std::wostream& operator<<(std::wostream& s, const cactiverow_t& row);   
   friend std::wostream& operator<<(std::wostream& s, const cactiverow_t* prow);




   
};//cactiverow_t

  template <typename TRow>
  std::string
  inline cactiverow_t::getscript_createtable (const std::string& tbname)
  {
      TRow::tName2Fields map=TRow::get_map_fields_static ();  
      TRow::tName2FieldsConstIter it;
      std::string result;
      
      result="create table "+tbname;
      result="DROP TABLE IF EXISTS "+tbname+ std::string(" cascade;");
      result=result+"CREATE TABLE "+tbname + std::string("("); 
      bool first=true;
      for (it=map.begin();it!=map.end();++it)
      {
         const tField& fld(it->second);
         if (!first)
         {
            result=result +",";
         }
         
         result=result+fld.fname()+" "+fld.ftype_dbname();
         if (first==true) first=false;
                           
      }//end of for
      
      result=result+");";      
      return result;
  }

  void
  inline
  cactiverow_t::register_field_setter_functor (const char* field_nm,
                                               const tFset__field fn,
                                             tMFset__field& map,
                                             boost::mutex& mtx,
                                             long long& initflag)
   {
      

      {//block to guard from repetead initializations
         boost::mutex::scoped_lock mylock(mtx);
         
         if (cactiverow_t::init_flag_is_ok_Setters(initflag))
         {

            return;
         }         
         else
         {
            //this is done at static init
            //cactiverow_t::init_flag_set_ok_Setters(initflag);
         }   
      }
      
      map.insert(std::make_pair(field_nm,fn));                                                   
   }                                             
  
   void 
   inline
   cactiverow_t::register_field_getter_functor (const char* field_nm,
                                                      const tFget__field fn,
                                                      tMFget__field& map,
                                                      boost::mutex& mtx,
                                                      long long& initflag)

   {

      {//block
         boost::mutex::scoped_lock mylock(mtx);
         
         if (cactiverow_t::init_flag_is_ok_Getters(initflag))
         {
            return;
         }         
         else
         {
            //this is done at static init
            //cactiverow_t::init_flag_set_ok_Getters(initflag);
         }   
      }

   
      map.insert(std::make_pair(field_nm,fn));                                                
   }


void
inline
cactiverow_t::register_field (const char* field_nm,
            cactiverow_t::tName2Fields& mymap,
            const int order,
            const int otl_fld_typ,
            const int field_length,
            const bool nullable,
            boost::mutex& mtx,
            long long& initflag)
   {


      {//block      
         boost::mutex::scoped_lock mylock(mtx);
         
         if (cactiverow_t::init_flag_is_ok_Fields(initflag))
         {
            return;
         }         
         else
         {
            //this is done at static init
            //cactiverow_t::init_flag_set_ok_Fields(initflag);
         }   
      }
   
      
      tField fld(
      order,      
      otl_fld_typ,
      field_length,
      nullable,          
      field_nm);

      mymap.insert(std::make_pair(field_nm,fld));   
   }


///create a set from multimap. Every element in the set
//is a tField, and since it is set -- it is ordered
//Eventually this can use some kind of cache instead
//of creating sets every time. However, compared to
//any database operations -- these ones are instantenious
cactiverow_t::tOrderedFields 
   inline 
   cactiverow_t::get_ordered_fields (const tName2Fields& field_map,
                                       const cactiverow_t& r)

{

   boost::mutex::scoped_lock mylock(r.get_activerow_static_mutex());

   
   
   cactiverow_t::tOrderedFields res;
   cactiverow_t::tName2FieldsConstIter it;
   for (it=field_map.begin();it!=field_map.end();++it)
   {
      res.insert(it->second);
   }
   
   return res;
}   

cactiverow_t::tOrderedFields 
   inline 
   cactiverow_t::get_some_ordered_fields (const tName2Fields& field_map,
                                          const std::vector<std::string>& names)

{
   cactiverow_t::tOrderedFields res;
   cactiverow_t::tName2FieldsConstIter it;
   
   std::vector<std::string>::const_iterator it_nm;

   for (it_nm=names.begin();it_nm!=names.end();++it_nm)
   {
      tName2FieldsConstIter f=  field_map.find(*it_nm);
      if (f!=field_map.end())
      {
            res.insert(f->second);
      }
   }      
   return res;
}   

 
 tField
 inline 
 cactiverow_t::asfield(const std::string& fname,
                         const tName2Fields& field_map)
    {   
      tName2FieldsConstIter pos=field_map.find(fname);
      if (pos!=field_map.end())
      {
         return (pos->second);
      }
      else
      {
         return tField::getdummy();
      }      
   }//end of asfield        


   
///  comma delimted list of bind names
// for example 
//:fldX<int>, :fldY<char[33]>  
std::string
inline
   cactiverow_t::get_bind_str (const cactiverow_t::tOrderedFields& fld_ordered)
{
   std::string result;   
   static const std::string prefix("_");
   static const size_t max_bind_nm_length=32;
   
   tOrderedFieldsConstIter b_it=fld_ordered.begin();
   tOrderedFieldsConstIter e_it=fld_ordered.end();
   bool first=true;
   while (b_it!=e_it) 
   { 
      //we do not want comma infront of first
      if (first==false)
      {
         result=result+",";
      }   
   
      //use f"ordernumber" instead of field name
      //if field name or field type will result in a string
      //exceeding 32 (because apparently that's max oracle allows
      //for bind variables (Sergey said that)
      std::string tmp;
      tmp=orm_var_type_name(b_it->fotltyp(),b_it->flen()) + b_it->fname();
      if (tmp.length()>max_bind_nm_length)
      {
         result=result+":"+prefix+"f"+
            boost::lexical_cast<std::string>(b_it->forder() );
      }
      else
      {   
         result=result+":"+prefix+b_it->fname(); 
      }  
      //vp use otl to completely defind bind type name
      //if (b_it->flen()==0) 
      { 
         result=result+ 
         "<"+orm_var_type_name(b_it->fotltyp(),b_it->flen()) 
         +">"; 
      } 
/*  vp use otl to completely define var bind name      
      else 
      { 
         //sergey said that strings are null terminated
         //therefore I have to account for another byte
         //and incase of oci, it is 2 more bytes
         result=result+ 
         "<"+orm_var_type_name(b_it->fotltyp()) 
         +"[" + boost::lexical_cast<std::string>(b_it->flen()+2)
         + "]>"; 
      } 
*/      
      ++b_it; 
      
      if (first==true) first=false;
    }
   return result;   
}   

///get a comma delimited list of fields for a given row
std::string
inline
   cactiverow_t::get_fieldslist_str (const cactiverow_t::tOrderedFields& fld_ordered,
               const std::string& prefix)
{
   std::string result;   
   
   
   tOrderedFieldsConstIter b_it=fld_ordered.begin();
   tOrderedFieldsConstIter e_it=fld_ordered.end();
   bool first=true;
   while (b_it!=e_it) 
   { 
      //we do not want comma infront of first
      if (first==false)
      {
         result=result+",";
      }   
      result=result+prefix+b_it->fname(); 
      ++b_it;       
      if (first==true) first=false;
    }
   return result;   
}   

///get a comma delimited list of fields for a given row 
//for use in SQL-OUT results meaning in places like
// select <result of this function >   from .....
//it is different than the above function because
//in here I use a list of conversions to force db
//to convert from one datatype to another 
std::string
inline
   cactiverow_t::get_fieldslist_str_sqlout
             (const cactiverow_t::tOrderedFields& fld_ordered,
               const std::string& prefix)
{
   std::string result;   
   
   
   tOrderedFieldsConstIter b_it=fld_ordered.begin();
   tOrderedFieldsConstIter e_it=fld_ordered.end();
   bool first=true;
   while (b_it!=e_it) 
   { 
      //we do not want comma infront of first
      if (first==false)
      {
         result=result+",";
      }   
      result=result+prefix+b_it->sqlout_fname(); 
      ++b_it;       
      if (first==true) first=false;
    }
   return result;   
}   





///get a comma delimited list of fields for a given row 
//for use in SQL-IN results meaning in places like
// select <result of this function >   from .....
//it is different than the above function because
//in here I use a list of conversions to force db
//to convert from one datatype to another 
//for example for DECIMAL I send strings, however
//database expects decimal
std::string
inline
   cactiverow_t::get_fieldslist_str_sqlin
             (const cactiverow_t::tOrderedFields& fld_ordered,
               const std::string& prefix)
{
   std::string result;   
   
   
   tOrderedFieldsConstIter b_it=fld_ordered.begin();
   tOrderedFieldsConstIter e_it=fld_ordered.end();
   bool first=true;
   while (b_it!=e_it) 
   { 
      //we do not want comma infront of first
      if (first==false)
      {
         result=result+",";
      }   
      result=result+prefix+b_it->sqlin_fname(); 
      ++b_it;       
      if (first==true) first=false;
    }
   return result;   
}   







   void 
   inline
   cactiverow_t::write_values_to_fields(const tOrderedFields& field_names,
                                  cactiverow_t& row,
                                  CFFunctorAssignVal& functor)
   {
      const tMFset__field& functions=get_map_setfunctors();      
      tOrderedFieldsConstIter it;
      for (it=field_names.begin();it!=field_names.end();++it)
      {
         const tField& field(*it);
         //the code is identical to the 
         //getter stuff, only map and map iterator types are different
         tMFset__fieldConstIter pos=functions.find(field.fname());
         if (pos!=functions.end())
         {
            //we found setter function
            try{
               pos->second(row,functor);
               }
             catch (otl_exception& e)
             {
               std::cerr<<e.msg<<std::endl;
             }  
         }
      } //end of for      
   }
                                     
  
   
   
   void 
   inline
   cactiverow_t::read_values_from_fields(const tOrderedFields& field_names, 
                                       const cactiverow_t& row,
                                       CFFunctorReceiveVal& functor) const
   {
      const tMFget__field& functions=get_map_getfunctors();      
      tOrderedFieldsConstIter it;
      for (it=field_names.begin();it!=field_names.end();++it)
      {
         const tField& field(*it);
         tMFget__fieldConstIter pos=functions.find(field.fname());
         if (pos!=functions.end())
         {
            //we found getter function
            pos->second(row,functor);
         }
      } //end of for   
   } //end of function read values                                   


   void 
   inline
   cactiverow_t::assign_otlnull (void)
   {
      CFFunctorSet__to_otlnull makenull;
      tOrderedFields fields=      
         cactiverow_t::get_ordered_fields (get_map_fields(),*this);
      write_values_to_fields (fields,
                              *this,
                              makenull);         
   }
  
   inline
   void 
   cactiverow_t::assign_otlnull_somefields (const std::vector<std::string>& names)
   {
      /* note that in fucntion of Any_Row we use the vritual version
      of get_map_fields, not the static version
      */
      CFFunctorSet__to_otlnull makenull;
      tOrderedFields fields=      
         cactiverow_t::get_some_ordered_fields (get_map_fields(),names);         
      write_values_to_fields (fields,
                              *this,
                              makenull);
   }
  




   void 
   inline
   cactiverow_t::assign_randomval (void)
   {
      CFFunctorSet__to_randomval makerandom;
      tOrderedFields fields=      
         cactiverow_t::get_ordered_fields (get_map_fields(),*this);
      write_values_to_fields (fields,
                              *this,
                              makerandom);         
   }
  
   inline
   void 
   cactiverow_t::assign_randomval_somefields (const std::vector<std::string>& names)
   {
      /* note that in function of Any_Row we use the virtual version
      of get_map_fields, not the static version
      */
      CFFunctorSet__to_randomval makerandom;
      tOrderedFields fields=      
         cactiverow_t::get_some_ordered_fields (get_map_fields(),names);         
      write_values_to_fields (fields,
                              *this,
                              makerandom);
   }

#if defined(USE_BOOST_SERIALIZE)

    template <typename Archive>
    inline
    void 
    cactiverow_t::serialize(Archive& ar, const unsigned int version)
    {
      /* note that in function of Any_Row we use the virtual version
      of get_map_fields, not the static version
      */
      CFFunctorSerializeFrom__boost<Archive> functor_serializeFrom(ar,version);
      tOrderedFields fields=      
        cactiverow_t::get_ordered_fields (get_map_fields(),*this);
      write_values_to_fields (fields,
                              *this,
                              functor_serializeFrom);


    }

/*
    template <typename Archive>
    inline
    void 
    cactiverow_t::serializeFrom__boost(Archive& ar, const unsigned int version)
    {

    }
*/

#endif //USE_BOOST_SERIALIZE






  
                                       
   //from stream to row
   inline
   otl_stream& 
   operator>>(otl_stream& s, cactiverow_t& row)
   {
   
   
      CFFunctorRead__otlstream functor(s);      
      cactiverow_t::tOrderedFields fields=      
      cactiverow_t::get_ordered_fields (row.get_map_fields(),row);         
      row.write_values_to_fields (fields,
                              row,
                              functor);   
      return s;
   }
   
   //from row to stream
   inline
   otl_stream& 
   operator<<(otl_stream& s, const cactiverow_t& row)
   {
      CFFunctorWrite__otlstream functor(s);      
      cactiverow_t::tOrderedFields fields=      
      cactiverow_t::get_ordered_fields (row.get_map_fields(),row);         
      row.read_values_from_fields (fields,
                              row,
                              functor);   
      return s;
   }

   inline
   otl_stream& 
   operator>>(otl_stream& s, cactiverow_t* prow)
   {
      return s>>*prow;
   }





   
   //from row to stream
   inline
   std::ostream& 
   operator<<(std::ostream& s, const cactiverow_t& row)
   {
       CFFunctorWrite__ostream functor(s);      
      cactiverow_t::tOrderedFields fields=      
      cactiverow_t::get_ordered_fields (row.get_map_fields(),row);         
      row.read_values_from_fields (fields,
                              row,
                              functor);   
      return s;
   }

   inline
   std::ostream& 
   operator<<(std::ostream& s, const cactiverow_t* prow)
   {
      s<<*prow;
      return s;
   }


   //from row to Wstream
   inline
   std::wostream& 
   operator<<(std::wostream& s, const cactiverow_t& row)
   {
       CFFunctorWrite__wostream functor(s);      
      cactiverow_t::tOrderedFields fields=      
      cactiverow_t::get_ordered_fields (row.get_map_fields(),row);         
      row.read_values_from_fields (fields,
                              row,
                              functor);   
      return s;
   }

   inline
   std::wostream& 
   operator<<(std::wostream& s, const cactiverow_t* prow)
   {
      s<<*prow;
      return s;
   }

/// Assign any cactive_field to any criteria
//via this Proxy mechanism
//As you can see the constructor takes the base
//class of the cactive_field
//
//that constructor is used implicitly
//when a cactivefield_t variable 
//is shifted into the any_criteria
struct ccriteria_field_t
{
  ccriteria_field_t( const tField& f,
                     const std::string& prefix="c",
                     const bool tTypecast=false)
  {
      //this trick with %d lets me use
      //sprintf to adjust the id of each variable
      if (prefix.length()>0)
      {      
         std::string complete_prefix=prefix+"_%d_";
         m_str= bindable_str(f,complete_prefix,tTypecast);
      }
      else
      {
         m_str= bindable_str(f,"",tTypecast);
      }      
  }

   ///call this constructor
   //if you want to make custom expressions
   //such that :varx<int>*2
   //you are on your own however, as far as
   //manging names/etc
  ccriteria_field_t(const std::string& customstr)
  {
      //this trick with %d lets me use
      //sprintf to adjust the id of each variable
      m_str= customstr;
  }


  
  ///this function is called from many places
  //so be careful modifying it
  static
  std::string bindable_str( const tField& f,
                           //%d is a trick that lets
                           //stream operators to generate
                           //unique prefixes by using sprintf
                            const std::string& fldpref="c",
                            const bool bTypecast=false)
  {   
      std::string m_str; 
      const int max_bind_nm_length=32;
      
      
      size_t bind_name_len=std::string(orm_var_type_name(f.fotltyp(),f.flen())).length() + f.fname().length();
      if (bind_name_len>max_bind_nm_length)
      {
         //just use the field order
         m_str=m_str+":"+fldpref+
            boost::lexical_cast<std::string>(f.forder());
      }
      else
      {  
         //use field name with a prefix
         //note that the prefix is different than in the 
         //function that returns 'bind string' for a row 
         m_str=m_str+":"+fldpref+f.fname(); 
      }  
//vp use otl to completely define the var type name      
//      if (f.flen()==0) 
      { 
         m_str=m_str+ 
         "<"+orm_var_type_name(f.fotltyp(),f.flen()) 
         +">"; 
      } 
/* vp use otl to comepletely define the var type name      
      else 
      { 
         m_str=m_str+ 
         "<"+orm_var_type_name(f.fotltyp())
         +"[" + boost::lexical_cast<std::string>(f.flen())
         + "]>"; 
      }
*/      
      if (bTypecast==true)
      {
         m_str=m_str+orm_fld_typecast(f.fotltyp(),f.flen());
      }         
      return m_str;//returning the local result string 
    }  
    
    //comma that allows us to do :ph1,ph2
    ccriteria_field_t& operator^ (const ccriteria_field_t& in)
    {
      if (m_str.length()<1)
      {
         m_str=m_str+","+in.dbname();
      }
      else
      {
         m_str=m_str+" ,"+in.dbname();
      }   
      return *this;
    }
    


    template<typename TVal>
    ccriteria_field_t& operator== (const ccriteria_field_t& in)
    {      
      m_str=m_str+"="+in.dbname();       
      return *this;
    }

    template<typename TVal>
    ccriteria_field_t& operator< (const ccriteria_field_t& in)
    {      
      m_str=m_str+"<"+in.dbname();       
      return *this;
    }


    template<typename TVal>
    ccriteria_field_t& operator> (const ccriteria_field_t& in)
    {      
      m_str=m_str+">"+in.dbname();       
      return *this;
    }

    template<typename TVal>
    ccriteria_field_t& operator!= (const ccriteria_field_t& in)
    {      
      m_str=m_str+"<>"+in.dbname();       
      return *this;
    }
    
    
    template<typename TVal>
    ccriteria_field_t& operator<= (const ccriteria_field_t& in)
    {      
      m_str=m_str+"<="+in.dbname();       
      return *this;
    }
    
    
    template<typename TVal>
    ccriteria_field_t& operator>= (const ccriteria_field_t& in)
    {      
      m_str=m_str+">="+in.dbname();       
      return *this;
    }



    
    
    
      const std::string& dbname(void)const {return m_str;}
   private:
     std::string m_str;
     
}; //end of findablef







///Local to the Database (or other storage mechanism)
// field
// 
struct ccriteria_local_field_t
{
   //I hope that the string passed in is indeed field name
   //but there isno way to verify
   //so this can be anything.
   //therefore this constructor can be used to inject any sql
   //do not supply data for it from user input or compromiseable db   
   ccriteria_local_field_t (const std::string& fieldnm,
           const std::string& tb="",
           const std::string& pref="")   
   {
      //pref can be anything legal in your db.
      if (tb.length()>0)
      {
         m_str=pref+tb+"."+fieldnm;      
      }
      else
      {
         m_str=pref+fieldnm;
      }
   }   
   
   //this allows to specify local fields but with table names infront
   //this constructor unlike the above forces a field to be a 
   //a tField or of type cactive_field. 
   //therefore this constructor forces a 'declared' name
   //therefore if the name would change int he row definition,
   //and this constructor is used, we will get a compiler error
   //and that's what we want: type safe FIELDS
   ccriteria_local_field_t ( const tField& f,
                             const std::string& tb="",
                             const std::string& pref="")
   {
   
      //pref can be anything legal in your db.
      if (tb.length()>0)
      {
         m_str=pref+tb+"."+f.fname();
      }
      else
      {
         m_str=pref+f.fname();
      }
   }                             


   //START operators that take in thesame
   ccriteria_local_field_t&
   operator==(const ccriteria_local_field_t& in)
   {
     m_str=m_str+" "+"="+" "+in.dbname();
     return *this;
   }

    ccriteria_local_field_t&
    operator<(const ccriteria_local_field_t& in)
   {
     m_str=m_str+" "+"<"+" "+in.dbname();
     return *this;
   }


    ccriteria_local_field_t&
    operator>(const ccriteria_local_field_t& in)
   {
     m_str=m_str+" "+">"+" "+in.dbname();
     return *this;
   }


    ccriteria_local_field_t&
    operator!=(const ccriteria_local_field_t& in)
   {
     m_str=m_str+" "+"<>"+" "+in.dbname();
     return *this;
   }



    ccriteria_local_field_t&
    operator<=(const ccriteria_local_field_t& in)
   {
     m_str=m_str+" "+"<="+" "+in.dbname();
     return *this;
   }


    ccriteria_local_field_t&
    operator>=(const ccriteria_local_field_t& in)
   {
     m_str=m_str+" "+">="+" "+in.dbname();
     return *this;
   }

   //comma
   ccriteria_local_field_t&
    operator^(const ccriteria_local_field_t& in)
   {
     m_str=m_str+" "+","+" "+in.dbname();
     return *this;
   }
   //END operators that take in thesame


   //START operators that take in BINDNMAME
   ccriteria_local_field_t&
   operator==(const ccriteria_field_t& in)
   {
     m_str=m_str+" "+"="+" "+in.dbname();
     return *this;
   }

    ccriteria_local_field_t&
    operator<(const ccriteria_field_t& in)
   {
     m_str=m_str+" "+"<"+" "+in.dbname();
     return *this;
   }


    ccriteria_local_field_t&
    operator>(const ccriteria_field_t& in)
   {
     m_str=m_str+" "+">"+" "+in.dbname();
     return *this;
   }


    ccriteria_local_field_t&
    operator!=(const ccriteria_field_t& in)
   {
     m_str=m_str+" "+"<>"+" "+in.dbname();
     return *this;
   }



    ccriteria_local_field_t&
    operator<=(const ccriteria_field_t& in)
   {
     m_str=m_str+" "+"<="+" "+in.dbname();
     return *this;
   }


    ccriteria_local_field_t&
    operator>=(const ccriteria_field_t& in)
   {
     m_str=m_str+" "+">="+" "+in.dbname();
     return *this;
   }

   //comma
   ccriteria_local_field_t&
    operator^(const ccriteria_field_t& in)
   {
     m_str=m_str+" "+","+" "+in.dbname();
     return *this;
   }
   //END operators that take in bindable name of a field



   //-------------------------------
   //START operators that take in string and others
   template <typename TVal>
   ccriteria_local_field_t&
   operator==(const TVal& in)
   {
   
     m_str=m_str+" "+"="+" "+boost::lexical_cast<std::string>(in);
     return *this;
   }

   template <typename TVal>
    ccriteria_local_field_t&
    operator<(const TVal& in)
   {
     m_str=m_str+" "+"<"+" "+boost::lexical_cast<std::string>(in);
     return *this;
   }

   template <typename TVal>
    ccriteria_local_field_t&
    operator>(const TVal& in)
   {
     m_str=m_str+" "+">"+" "+boost::lexical_cast<std::string>(in);
     return *this;
   }


   template <typename TVal>
    ccriteria_local_field_t&
    operator!=(const TVal& in)
   {
     m_str=m_str+" "+"<>"+" "+boost::lexical_cast<std::string>(in);
     return *this;
   }


   template <typename TVal>
    ccriteria_local_field_t&
    operator<=(const TVal& in)
   {
     m_str=m_str+" "+"<="+" "+boost::lexical_cast<std::string>(in);
     return *this;
   }

   template <typename TVal>
    ccriteria_local_field_t&
    operator>=(const TVal& in)
   {
     m_str=m_str+" "+">="+" "+boost::lexical_cast<std::string>(in);
     return *this;
   }

   //comma
   template <typename TVal>   
   ccriteria_local_field_t&
    operator^(const TVal& in)
   {
     m_str=m_str+" "+","+" "+boost::lexical_cast<std::string>(in);
     return *this;
   }
   //END operators that take in string and others






   
   const std::string& dbname()const
   {
      return m_str;
   }
   private:
      std::string m_str;
   
};








///Variables representing field names
//are declared as this type

//my trick macro that essentially allows to pass
//a string representing field name as a non-class type template argument
//it is done by defining a function within definition
//of a class and then passing a pointer of that static
//function as a template argument
//Probably another way would be to define a new class
//for each field, and in the constructor of that class
//have hacrdocded the name of the field

#define DECL_OTL_FIELD(fld_order,cpp_typ,otl_typ,fld_nm,fld_len) \
static char *fld_nm##__nm (void) \
{  \
   static char r[] =#fld_nm; \
   return r; \
} \
\
template <typename TInst> \
static otl_value<cpp_typ>& get_fld_ref__##fld_nm (TInst& inst) \
{ \
   return inst.##fld_nm ; \
} \
\
/* return const cpp value */\
const cpp_typ & fld_nm##__valc (void) const\
{ \
   return fld_nm##.v ; \
} \
\
/*return non cost cpp value reference*/\
cpp_typ & fld_nm##__val (void)\
{ \
   return fld_nm##.v ; \
} \
\
\
\
template <typename TInst> \
static const otl_value<cpp_typ>& get_fld_ref__##fld_nm (const TInst& inst)\
{ \
   return inst.##fld_nm ; \
} \
\
\
\
/* assign value to a field by calling a functor (and sending the field to it) */\
static void set_field_fromfunctor__##fld_nm (cactiverow_t& row, CFFunctorAssignVal& f) \
{\
   tThisClass& r(dynamic_cast<tThisClass&>(row));\
   f.operator()<tAttrType__##fld_nm>(r.##fld_nm);\
}\
\
\
static void give_field_tofunctor__##fld_nm (const cactiverow_t& row, CFFunctorReceiveVal& f) \
{\
   const tThisClass& r(dynamic_cast<const tThisClass&>(row));\
   f.operator()<const tAttrType__##fld_nm>(r.##fld_nm);\
}\
/* now typedef the field type so that it can be used*/\
/*when the user of the class knows exactly a field name*/\
/*and simply wants to create another variable of the same*/\
/* type*/\
\
\
   typedef otl_value<cpp_typ> \
   tOTLAttrType__##fld_nm; \
   \
   \
   typedef \
   cactivefield_t<cpp_typ,\
fld_order,\
fld_len,\
otl_typ,\
&m_field_map,\
&fld_nm##__nm,\
&set_field_fromfunctor__##fld_nm,\
&m_functorset_field_map,\
&give_field_tofunctor__##fld_nm,\
&m_functorget_field_map,\
&static_members_mutex,\
&init_flag\
>   tAttrType__##fld_nm; \
\
 \
\
\
/*now declare the field */\
tAttrType__##fld_nm fld_nm ;\
\
\
/*get field as 'bindable name'  */\
static \
ccriteria_field_t fld_nm##__bn(const bool typecast=false)\
{\
  tField tf(fld_order,\
            otl_typ,\
            fld_len,\
            true,\
            fld_nm##__nm());\
            /*passing empty string indicates that I do not want a prefix*/\
  return ccriteria_field_t(tf,"",typecast);\
}\
\
/*get field as 'local' field */\
static \
ccriteria_local_field_t fld_nm##__lf(const std::string& tbnm="")\
{\
  tField tf(fld_order,\
            otl_typ,\
            fld_len,\
            true,\
            fld_nm##__nm());\
            \
  ccriteria_local_field_t res(tf,tbnm);\
  return res;\
}
  


/// generate a type of a field for a given row
#define T_FLD(class_nm,fld_nm) class_nm##::##tAttrType__##fld_nm

//generate a C++ value type given class and field
#define T_VALT(class_nm,fld_nm) class_nm##::##tAttrType__##fld_nm::tValTyp




#define DECL_OTL_FIELD_UNIQUE_KEY1(fld_nm1) \
typedef tAttrType__##fld_nm1  unique_t ;\
const unique_t& get_unique_key1(void) const\
{\
   return fld_nm1;\
   \
}\
\
\
bool operator<(const tThisClass& in) const\
{\
   return fld_nm1 < in.##fld_nm1;\
}


//put this into your cpp file
#define DECL_OTL_ROW_CPP(classnm) \
   boost::mutex classnm##::static_members_mutex;\
   long long classnm##::init_flag=0LL;\
   classnm##::tName2Fields classnm##::m_field_map;\
   classnm##::tMFset__field classnm##::m_functorset_field_map;\
   classnm##::tMFget__field classnm##::m_functorget_field_map;\
   \
   /*class constructor */\
   classnm##::##classnm (const bool isStatic)\
   {\
      /*now announce to the world that we have initialized the row*/\
      /*this is critical to happen during static initialization stage*/\
      /*when we are in constructor all fields have been created statically*/\
      if (isStatic)\
      {\
         cactiverow_t::init_flag_set_ok_Fields(classnm##::init_flag);\
         cactiverow_t::init_flag_set_ok_Setters(classnm##::init_flag);\
         cactiverow_t::init_flag_set_ok_Getters(classnm##::init_flag);\
      }\
   }\
   ##classnm##::~##classnm (void)\
   {\
      /*std::cout<<#classnm<<" destructor"<<std::endl;*/\
      /*std::cout<<#classnm<<" set map size was: "<<m_functorset_field_map.size()<<std::endl;*/\
      /*std::cout<<#classnm<<" get map size was: "<<m_functorget_field_map.size()<<std::endl;*/\
      /*printf("%s get map size was %d \n",#classnm,m_functorget_field_map.size());*/\
   }\
   /*create instance of this row, so that static maps are populated */\
   classnm classnm##_staticROW(true);
   
 
   
//put this into your headerfile
//inside the row def
#define DECL_OTL_ROW_HPP(classnm) \
      static boost::mutex static_members_mutex;\
      /*initialization bit vector to tell me what static regi */\
      /*strations have been done */\
      /*this flag can be modified only with a mutex */\
      static long long  init_flag;\
      typedef classnm tThisClass; \
      typedef boost::shared_ptr< classnm > tThisClassSharedPtr;\
      /* constructor/destructor */\
      classnm (bool isStatic=false);\
      ~##classnm (void);\
      struct classnm##__lessthan \
      {\
         bool operator () (const tThisClassSharedPtr a, \
               const tThisClassSharedPtr b) \
         {\
            return (*a)<(*b);\
         }\
      };\
      typedef std::set<tThisClassSharedPtr,classnm##__lessthan> tThisClassSet;\
      typedef std::set<tThisClassSharedPtr,classnm##__lessthan>::const_iterator tThisClassSetConstIter;\
      typedef std::set<tThisClassSharedPtr,classnm##__lessthan>::iterator tThisClassSetIter;\
      \
      typedef std::multiset<tThisClassSharedPtr,classnm##__lessthan> tThisClassMSet;\
      typedef std::multiset<tThisClassSharedPtr,classnm##__lessthan>::const_iterator tThisClassMSetConstIter;\
      typedef std::multiset<tThisClassSharedPtr,classnm##__lessthan>::iterator tThisClassMSetIter;\
      \
      typedef std::vector<tThisClassSharedPtr> tThisClassVector;\
      typedef std::vector<tThisClassSharedPtr>::const_iterator tThisClassVectorConstIter;\
      typedef std::vector<tThisClassSharedPtr>::iterator tThisClassVectorIter;\
      \
\
      /*static array containing tField values*/\
      /*for every field name declared with*/\
      /*DECL_OTL_FIELD*/\
      /*when we access this array it should me mutexed*/\
      static tName2Fields m_field_map;\
\
      /*static array containing pointers to static*/\
      /*functions that return field value given*/\
      /*field name.  For the fields declared with DECL_OTL_FIELD*/\
      static tMFset__field m_functorset_field_map;    \
      \
      static tMFget__field m_functorget_field_map;\
/*use the static version of the function when accessing data outside this class hierarchy*/\
      static const tName2Fields& get_map_fields_static (void) \
      {\
         return m_field_map;\
      }\
\
\
/*use the member function when accessing data from CDBA_Any*/\
      virtual const tName2Fields& get_map_fields (void) const \
      {\
         return m_field_map;\
      }\
\
   /*get the map representing the 'getters' of the field */\
   virtual tMFget__field& get_map_getfunctors (void)\
   {\
         return m_functorget_field_map;\
   }\
\
   virtual const tMFget__field& get_map_getfunctors (void) const\
   {\
         return m_functorget_field_map;\
   }\
\
   /*get the map that stores the 'setters' as relates to field */\
   virtual tMFset__field& get_map_setfunctors (void)\
   {      \
      return m_functorset_field_map;\
   }\
\
   virtual const tMFset__field& get_map_setfunctors (void)const\
   {      \
      return m_functorset_field_map;\
   }\
   virtual boost::mutex& get_activerow_static_mutex (void) const\
   {\
      return static_members_mutex;\
   }\
   virtual long long& get_activerow_static_initflag (void) const\
   {\
      return init_flag;\
   }\
   static tOrderedFields get_ordered_fields_static (void)\
   {\
      boost::mutex::scoped_lock mylock(static_members_mutex);\
      cactiverow_t::tOrderedFields res;\
      cactiverow_t::tName2FieldsConstIter it;\
      const cactiverow_t::tName2Fields& field_map(classnm##::get_map_fields_static());\
      for (it=field_map.begin();it!=field_map.end();++it)\
      {\
         res.insert(it->second);\
      }\
      return res;\
   }


//end of macro

//dummy default functions that we use as template arguments
//there is no implementation for them
//so if they get called you will get link error 
//and that means that you forgot to pass real functions
//as pointers to the template arguments

char* get_char_ptr (void);


template <typename T, 
         int fldorder,
         int fldlen,
         int ftype, 
         cactiverow_t::tName2Fields* ptr_static_field_map,
         char* (*_fn)(void),
         cactiverow_t::tFset__field field_setter,                                        
         cactiverow_t::tMFset__field* map_of_field_setters,
         cactiverow_t::tFget__field field_getter,         
         cactiverow_t::tMFget__field* map_of_field_getters,
         boost::mutex* static_mtx_ptr,
         long long* initflag                                            
         > 
struct cactivefield_t: public otl_value<T>,
   public tField,
   public boost::enable_shared_from_this<cactivefield_t<
   T,
   fldorder,
   fldlen,
   ftype, 
   ptr_static_field_map,
   _fn,
   field_setter,                                        
   map_of_field_setters,
   field_getter,         
   map_of_field_getters,
   static_mtx_ptr,
   initflag
   > >
{

   //constructor
   cactivefield_t (void)
      :otl_value<T>(),
      tField(fldorder,ftype,fldlen,true,_fn()),
      
      m_ref_field_map(*ptr_static_field_map),      
      m_field_name_func_ptr(_fn),
      
      m_field_setter(field_setter),  
      m_map_of_field_setters(*map_of_field_setters),
      m_field_getter (field_getter),
      m_map_of_field_getters(*map_of_field_getters),
      
      m_default(false)
      
      
      {        
       //now register field name in static map
       //this call must be thread safe
       
       /* register field into the static map */
       cactiverow_t::register_field
       /*_fr*/(_fn(),
            *ptr_static_field_map,
            fldorder,
            ftype,
            fldlen,
            true,
            *static_mtx_ptr,
            *initflag);
            
      /* register the field getters into the static map */
      
      
      cactiverow_t::register_field_setter_functor
      (_fn(),field_setter,*map_of_field_setters,*static_mtx_ptr,*initflag);
      
      cactiverow_t::register_field_getter_functor
      (_fn(),field_getter,*map_of_field_getters,*static_mtx_ptr,*initflag);

      }//end of void constructor
      
   cactivefield_t (const otl_value<T>& in)
      :otl_value<T>(in),
      tField(fldorder,ftype,fldlen,true,_fn()),
      m_ref_field_map(*ptr_static_field_map),
      
      m_field_name_func_ptr(_fn),
      
      m_field_setter(field_setter),  
      m_map_of_field_setters(*map_of_field_setters),
      m_field_getter (field_getter),
      m_map_of_field_getters(*map_of_field_getters),
      
      m_default(false)
      
      
      { 
       
       //now register field name in static map
       //this call must be thread safe
       
       /* register field into the static map */
       cactiverow_t::register_field
       (_fn(),
            *ptr_static_field_map,
            fldorder,
            ftype,
            fldlen,
            true,
            *static_mtx_ptr,
            *initflag);
            
      /* register the field getters into the static map */
      
      
      cactiverow_t::register_field_setter_functor
      (_fn(),field_setter,*map_of_field_setters,*static_mtx_ptr,*initflag);
      
      cactiverow_t::register_field_getter_functor
      (_fn(),field_getter,*map_of_field_getters,*static_mtx_ptr,*initflag);

      }//end of otl_value type constructor
      
      
      
      
      
      
      
      
   //define the value type of what was passed in 
   //as a first template argument
   //this way that template argument can be used in typedefs
   typedef T  tValTyp;       
   
   typedef 
   cactivefield_t<
   tValTyp,
   fldorder,
   fldlen,
   ftype, 
   ptr_static_field_map,
   _fn,
   field_setter,                                        
   map_of_field_setters,
   field_getter,         
   map_of_field_getters,
   static_mtx_ptr,
   initflag
   >   tThisClass;
   
   typedef boost::shared_ptr<tThisClass> tThisClassSharedPtr;
   typedef std::set<tThisClassSharedPtr> tThisClassSet;
   typedef std::vector<tThisClassSharedPtr> tThisClassVector;
   typedef std::map<std::string,tThisClassSharedPtr> tThisClassStrMap;
     

#if defined (BOOST_SERIALIZATION_SERIALIZATION_HPP)
   boost::archive::detail::basic_oarchive& 
      operator& (boost::archive::detail::basic_oarchive& ar)
   {
      ar & v;
      return ar;
   }
   
   boost::archive::detail::basic_oarchive& 
    fromself (boost::archive::detail::basic_oarchive& ar) const
   {
      ar&v;
      return ar;
   }
   
#endif   
         
   tField asfield (void) const
   {
      
      return (tField)(*this);
   }
   

   
   //end of typecast operators
   
   
   
   tThisClass& copy(const tThisClass& in)
   {
      //not implemented
      assert(0);   
   }
   
   
   //we only want to copy values, not field attributes
   //if you want to copy everything use copy
   
   tThisClass& operator = (const tThisClass& in)
   {
      //this invokes a typecast (defined in the parent)
      //to the class T
      this->otl_value<T>::operator=(in);
      
      m_default=in.m_default;
      return *this;
   }
   
   
   
  
   /* this operator insures that we only copy 
      values and not the field attributes */
      
   tThisClass& operator = (const otl_value<T>& in)
   {
      this->otl_value<T>::operator=(in);
      
      m_default=false;
      return *this;
   }
   
   
   bool operator < (const tThisClass& in) const
   {
      //if null or 'default' it ceretainly not 'less than'
      //anything
      if (is_null ())
      {
         return false;
      }
      
      if (m_default==true)
      {
         return false;
      }
      
      
      return this->v < in.v;
   }
   
   bool operator < (const otl_value<T>& in) const
   {
      if (is_null() || is_default()) return false;
   
      return this->v < in.v;
   }

   //this operator is probably not necessary
   bool operator < (const T& in) const
   {
      if (is_null() || is_default()) return false;
      return this->v < in;
   }
   
   //our is equal operators compare values only
   //because the types are compared at compile time
   //and different cdba type == will simply not compile
   //so if a programmer will want to compare values
   //he will have to make both of the same type or
   //typecast down to otl_value or value before the
   //comparasing
   bool operator== (tThisClass& in) const
   {
      if (  (is_null()== true && in.is_null()==true) ||
               (is_default()==true && in.is_default()==true)
          )
      {
         return true;
      }          
      
      if (  (is_null()==true)  && (in.is_null()==false) )
      {
         return false;
      }
      
      
      
      if (  (is_default()==true)  && (in.is_default()==false) )
      {
         return false;
      }
      
      
   
     if ((otl_value<T>::operator==(in))==false) return false;
/*     
     if (m_ftype!=in.m_ftype) return false;     
     if (m_forder!=m_forder) return false;
     if (m_fnullable!=m_fnullable)return false;
     if (m_flength!=m_flength)return false;
     if (m_fname!=m_fname) return false;
*/     
     
     return true;
   }
   
   
   bool operator== (const otl_value<T>& in) const
   {
      return (this->v==in.v);
   }
   

   bool operator== (const T& in) const
   {
      return (this->v==in);
   }


   
   void assign_any (const boost::any & in)
   {      
      
   
      /*
      std::cout<<" v type name is: "<<typeid(v).name()<<std::endl;
      std::cout<<" in type name is: "<<in.type().name()<<std::endl;
      */
   
   
      //the any can be constructed either with the basic type
      //that is acceptable to otl_value
      //or as otl_value<basic type>
      
      //if it is neither then we have an error
      //so we will attempt two type casts and 
      //if none of the succeed we will throw runtime error  
    
      try
      {         
         this->v=boost::any_cast<T>(in);
      }
      catch (boost::bad_any_cast& /*e*/)
      {
         try{
            this->otl_value<T>::operator=(
            boost::any_cast<otl_value<T> >(in));
         }
         catch (boost::bad_any_cast& e)
         {
            //ok if that did not work, now
            //I can use / approximations
            //for example
            /* it is ok to from int to int64 */
            /* but I have to write lots of code to do this*/
            
            /* so in the mean time, if you know that it is
            ok to type cast
            first type cast your any into the 'good type' and
            then pass this good type into regular assignment 
            operator not into this assign_any
            */
            throw e;
         }    
      }
      
      //if successfull
      m_default=false;   
   }
   
   void set_default (void)
   {
      m_default=true;
      otl_value::set_non_null();
   }
   
   void set_null (void)
   {
      m_default=false;
      otl_value::set_null();
   }
   
   bool is_default( void) const
   {
      return m_default;
   }
   
   private:
      //shows if the field must use the DB field 'default' value
      bool m_default;
   public:      
   
   
   
   
   const cactiverow_t::tName2Fields& m_ref_field_map;
   
   char* (*m_field_name_func_ptr)(void);
   
   //function pointer
   const cactiverow_t::tFset__field m_field_setter;                               
   //std map
   const cactiverow_t::tMFset__field& m_map_of_field_setters;
   //function pointer
   const cactiverow_t::tFget__field m_field_getter;         
   //std map
   const cactiverow_t::tMFget__field& m_map_of_field_getters;                                               
      
};//end of class


struct op
{

   struct EQ
   {
     std::string dbname(void)const{return std::string("=");}
   };


   struct NEQ
   {
     std::string dbname(void)const{return std::string("<>");}
   };



   struct LT
   {
     std::string dbname(void)const{return std::string("<");}
   };

   struct GT
   {
     std::string dbname(void)const{return std::string(">");}
   };


   struct AND
   {
     std::string dbname(void)const{return std::string("and");}
   };
   
   struct OR
   {
      std::string dbname(void)const{return std::string("or");}
   };
   
   struct WHERE
   {
      std::string dbname(void)const{return std::string("where");}
   };

   struct ISIN
   {
      std::string dbname(void) const{return std::string("in");}
   };
   struct NOTIN
   {
      std::string dbname(void)const{return std::string("not in");}
   };

  struct ISNOTNULL
   {
      std::string dbname(void)const{return std::string("IS NOT NULL");}
   };


  struct ISNULL
   {
      std::string dbname(void)const{return std::string("IS NULL");}
   };


   
   //open parenthese
   struct OP
   {
      std::string dbname(void)const{return std::string("(");}
   };

   struct CP
   {
      std::string dbname(void)const{return std::string(")");}
   };
  
};





   
   

/// How do you say for example:
//Insert into xyz (some_field) :values("abc")
//In otherwords you want to say that 'this field'
//has this value, and it is not an 'assignment' in a select
//
// Well, that's why I have this val2nm
//
//obviously if the some_field does not have a 'hardcoded value
//and instead is expected to be supplied to the olt stream, then
//do not use this class and instead use the ccriteria_field_t
//
template <typename TRow>
struct ccriteria_val2nm_t
{
   template <typename TValType>
   ccriteria_val2nm_t (const std::string& fieldnm,
                 const TValType& inval)
            :m_field(TRow::asfield(fieldnm,TRow::get_map_fields_static())),
            m_prefix("")            
   {  
      std::string quotes;
      if ( (m_field.fotltyp()>=8 && m_field.fotltyp()<=19)||
            m_field.fotltyp()==otl_var_char)
      {
          quotes="'";
      }
      else
      {
         quotes="";
      }          
      
      m_val_str=quotes+boost::lexical_cast<std::string>(inval)+quotes;
   }   
   const std::string dbname(void)const 
   {
      return   ccriteria_field_t::bindable_str(
                  m_field,                  
                  m_prefix);
    }
    
   const tField& field(void) const {return m_field;}
   const std::string& value(void) const { return m_val_str;}
   
   private:     
     tField m_field;
     std::string m_val_str;
     std::string m_prefix;
};//end of ccriteria_field_t



/* BINDABLE field name 
   in otherwords, just field name with ':' in front
   
   This is a runtime-typesafe way of doing it, because
   even though takes a field name as an argument,
   it actually finds and makes sure it exists within
   the row
   
   You may ask, well, how is this different then
   ccriteria_field_t
   
   The answer is that this class has different constructor
   signature, and IT MUST BE instanciated as a template
   where the template is a Active Row.
   
   ccriteria_field_nm<MyARow>("field_namexyz")   
   
 */   
   template <typename TRow>
struct ccriteria_fieldnm_t
{
   ccriteria_fieldnm_t (const std::string& fieldnm)
            :m_prefix(""),
            m_field(TRow::asfield(fieldnm,TRow::get_map_fields_static()))
   {      
   }   
   const std::string dbname(void)const 
   {
      return   ccriteria_field_t::bindable_str(
                  m_field,                  
                  m_prefix);
    }
   const tField& field(void) const {return m_field;};
   
   private:     
     tField m_field;
     std::string m_prefix;
};//end of ccriteria_fieldnm_t



   
   

template <typename TRow>
struct
cinsert_criteria_t
{

   //ccriteria_val2nm_t  means value is supplied rightway
   virtual  
   cinsert_criteria_t& operator<< (const ccriteria_val2nm_t<TRow>& bvf)
   {
      if (m_str.length()<1)
      {
         //append bvf.field().sqlin_fname() if there we want
         //to explicit typecasts for every field (it will be associated
         //with bound variable names not the column list as I thought before
         m_str=m_str+bvf.value();
      }
      else
      {
         //append bvf.field().sqlin_fname() if there we want
         //to explicit typecasts for every field (it will be associated
         //with bound variable names not the column list as I thought before      
         m_str=m_str+","+bvf.value();
      }
      
      //---------------------------
      if  (m_str_fld.length()<1)
      {
         m_str_fld=bvf.field().fname();
      }  
      else
      {       
         m_str_fld=m_str_fld+","+bvf.field().fname();
      }
      return *this;
   }
   
   

   //bindable meaning that the data will be supplied
   //by the program
   virtual
   cinsert_criteria_t& operator<< (const ccriteria_fieldnm_t<TRow>& bf)
   {      
      
      if (m_str.length()<1)
      {
         m_str=m_str+bf.dbname();
      }
      else
      {
         m_str=m_str+","+bf.dbname();
      }
      
      
      if  (m_str_fld.length()<1)
      {
         m_str_fld=bf.field().fname();
      }  
      else
      {
         m_str_fld=m_str_fld+","+bf.field().fname();
      }
      return *this;
   }

   ///This allows us to insert actual values
/*   I am not sure where I need to insert a value without a corresponding
     field in the field list
     
   template <typename TVal>
   cinsert_criteria_t& operator <<(const TVal& in)
   {
      std::string v=boost::lexical_cast<std::string>(in);
      m_str=m_str+","+v;
      return *this;      
   }
*/   

   
   virtual const std::string& get_fields_str(void) const
   {
      return m_str_fld;
   }


   virtual const std::string& get_val_str(void) const
   {
      return m_str;
   }

   virtual ~cinsert_criteria_t (void)
   {
   }

   private:
      std::string m_str;
      std::string m_str_fld;
};


///Base select criteria
/* here are some concepts

   I have three types of fields
   
   local to the database field
   
   bindable -- where field type is supplied from tField structure
   (at runtime)
   
   bindable -- where field type is supplied at compile time
      (by means of template instanciation)
      
      
   Those fields are
   
   
      ccriteria_field_t
      ccriteria_local_field_t
      ccriteria_fieldnm_t
      
      
      so the field_t and fieldnm_t are very close to each other
      because both get translated into :ph333 etc
      but how the type info is supplied is different
      
      of course I allow the 'anystr' to supplement any 
      custom SQL strings, but watch for sql injections there...
      meaning make sure that those types of strings do not
      come from user entered data.
      

*/

struct
cactive_criteria_t
{
   cactive_criteria_t (void)
   {
   }
   
   cactive_criteria_t (cactive_criteria_t& in)
   {
      this->operator<<(in.get_db_str());
   }
   
   cactive_criteria_t& operator=(const cactive_criteria_t& in)
   {
      this->operator<<(in.get_db_str());         
      return *this;      
   }

   cactive_criteria_t& operator=(const ccriteria_local_field_t& lf)
   {
      this->operator<<(lf);
      return *this;      
   }
   


   cactive_criteria_t& operator=(const ccriteria_field_t& bf)
   {
      this->operator<<(bf);
      return *this;      
   }

   
   
   
   //start the criteria
   //chain this instance with a bindable field
   //that is something that looks like :<fieldnm>
   cactive_criteria_t& start(const ccriteria_field_t& in)
   {
      this->operator<<(in);
      return *this;
   }
   
   //chain this instance with the local (local to database)
   //field name
   cactive_criteria_t& start (const ccriteria_local_field_t& lf)
   {
      this->operator<<(lf);
      return *this;
   }
   
   
   
   
   cactive_criteria_t& start (const std::string& anystr)
   {
      this->anys(anystr);
      return *this;
   }
   
   
   

   //chain this instance with 
   //any string
   //be careful as this will get passed to SQL
   //so injection attacks are possible
   //(I will add something later to 'securitize' this
   //
   //also wide character strings cannot be accepted here
   //because most DB drivers do not like SQL that has
   //any non-ascii encoding
   cactive_criteria_t& anys (const std::string& anystr)
   {
         this->operator<<(std::string(" ")+anystr);                  
         return *this;
   }



   //chain this instance with the local (local to database)
   //field name
   cactive_criteria_t& fld (const ccriteria_local_field_t& lf)
   {
      this->operator<<(lf);
      return *this;
   }


   //not strictly necessary but just adds 'symmetry'
   cactive_criteria_t& end (const std::string& anystr=std::string(""))
   {
      this->anys(anystr);
      return *this;
   }

   
   
   
   /* I do not know what to do with the functor yet
   const cactive_criteria_t& operator()(void) const
   {
   }
   */
   
   
   
   cactive_criteria_t& operator<< (const op::AND& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }
   

   cactive_criteria_t& operator<< (const op::OR& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }
   
   cactive_criteria_t& operator<< (const op::WHERE& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }


   cactive_criteria_t& operator<< (const op::NOTIN& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }

   cactive_criteria_t& operator<< (const op::ISNOTNULL& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }


   cactive_criteria_t& operator<< (const op::ISNULL& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }


   
   cactive_criteria_t& operator<< (const op::ISIN& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }


   cactive_criteria_t& operator<< (const op::OP& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }

   cactive_criteria_t& operator<< (const op::CP& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }


   cactive_criteria_t& operator<< (const op::EQ& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }
   
   cactive_criteria_t& operator<< (const op::NEQ& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";
      return *this;
   }

   
   
   cactive_criteria_t& operator<< (const op::LT& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";      
      return *this;
   }
   
   
   cactive_criteria_t& operator<< (const op::GT& op) 
   {
      m_str=m_str+" "+op.dbname()+" ";      
      return *this;
   }




   //start criteria logic operators   
   cactive_criteria_t& operator&&(const cactive_criteria_t& in)
   {
      this->operator<<(op::AND());
      m_str=m_str+" "+in.get_db_str();
      return *this;      
   }
      

   cactive_criteria_t& operator||(const cactive_criteria_t& in)
   {
      this->operator<<(op::OR());
      m_str=m_str+" "+in.get_db_str();
      return *this;      
   }


   cactive_criteria_t& operator!=(const cactive_criteria_t& in)
   {
      this->operator<<(op::NEQ());
      m_str=m_str+" "+in.get_db_str();
      return *this;      
   }
   
   //I am not sure if it is safe to provide this operator
   
   cactive_criteria_t& operator==(const cactive_criteria_t& in)
   {
      this->operator<<(op::EQ());
      m_str=m_str+" "+in.get_db_str();
      return *this;      
   }
   
   cactive_criteria_t& operator== (const ccriteria_local_field_t& lf)
   {
      this->operator<<(op::EQ());
      this->operator<<(lf);
      return *this;
   }
   
                                  
   
   
   //end criteria logic operators






   //another way of doing the same things as above   
   cactive_criteria_t& LT (const cactive_criteria_t& in) 
   {
      this->operator<<(op::LT());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }

   //another way of doing GT
   cactive_criteria_t& GT (const cactive_criteria_t& in) 
   {
      this->operator<<(op::LT());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }


   //another way of doing AND
   cactive_criteria_t& AND (const cactive_criteria_t& in) 
   {
      this->operator<<(op::AND());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }


   //another way of doing OR
   cactive_criteria_t& OR (const cactive_criteria_t& in) 
   {
      this->operator<<(op::OR());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }


   //another way of doing NEQ
   cactive_criteria_t& NEQ (const cactive_criteria_t& in) 
   {
      this->operator<<(op::NEQ());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }


   
   //another way of doing EQ
   cactive_criteria_t& EQ (const cactive_criteria_t& in) 
   {
      this->operator<<(op::EQ());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }

   //another way of doing OP
   cactive_criteria_t& OP (const cactive_criteria_t& in) 
   {
      this->operator<<(op::OP());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }


   //another way of doing close parenthesis
   cactive_criteria_t& CP (const cactive_criteria_t& in) 
   {
      this->operator<<(op::CP());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }


   //another way of doing NOTIN
   cactive_criteria_t& NOTIN (const cactive_criteria_t& in) 
   {
      this->operator<<(op::NOTIN());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }
   
   
   //another way of doing IN
   cactive_criteria_t& ISIN (const cactive_criteria_t& in) 
   {
      this->operator<<(op::ISIN());
      m_str=m_str+" "+in.get_db_str();
      return *this;
   }


   //end of CRITERIA to CRITERIA logic OPS
   
   
   //START CRITERIA TO LOCAL FIELDS logic OPS     
   cactive_criteria_t& LT (const ccriteria_local_field_t& in) 
   {
      this->operator<<(op::LT());
      this->operator<<(in);      
      return *this;
   }

   cactive_criteria_t& GT (const ccriteria_local_field_t& in) 
   {
      this->operator<<(op::LT());
      this->operator<<(in);
      return *this;
   }

   cactive_criteria_t& AND (const ccriteria_local_field_t& in) 
   {
      this->operator<<(op::AND());
      this->operator<<(in);
      return *this;
   }

   cactive_criteria_t& OR (const ccriteria_local_field_t& in) 
   {
      this->operator<<(op::OR());
      this->operator<<(in);
      return *this;
   }

   cactive_criteria_t& NEQ (const ccriteria_local_field_t& in) 
   {
      this->operator<<(op::NEQ());
      this->operator<<(in);
      return *this;
   }


   cactive_criteria_t& EQ (const ccriteria_local_field_t& in) 
   {
      this->operator<<(op::EQ());
      this->operator<<(in);
      return *this;
   }

   cactive_criteria_t& OP (const ccriteria_local_field_t& in) 
   {
      this->operator<<(op::OP());
      this->operator<<(in);
      return *this;
   }

   cactive_criteria_t& CP (const ccriteria_local_field_t& in) 
   {
      this->operator<<(op::CP());
      this->operator<<(in);
      return *this;
   }
   //END OF CRITERIA TO FIELDS logic OPS





   //START CRITERIA TO BOUND FIELDS logic OPS     
   template<typename TRow>
   cactive_criteria_t& LT (const ccriteria_fieldnm_t<TRow>& in) 
   {
      this->operator<<(op::LT());
      this->operator<<(in);      
      return *this;
   }

   template<typename TRow>
   cactive_criteria_t& GT (const ccriteria_fieldnm_t<TRow>& in) 
   {
      this->operator<<(op::GT());
      this->operator<<(in);
      return *this;
   }

   template<typename TRow>
   cactive_criteria_t& AND (const ccriteria_fieldnm_t<TRow>& in) 
   {
      this->operator<<(op::AND());
      this->operator<<(in);
      return *this;
   }

   template<typename TRow>
   cactive_criteria_t& OR (const ccriteria_fieldnm_t<TRow>& in) 
   {
      this->operator<<(op::OR());
      this->operator<<(in);
      return *this;
   }

   template<typename TRow>
   cactive_criteria_t& NEQ (const ccriteria_fieldnm_t<TRow>& in) 
   {
      this->operator<<(op::NEQ());
      this->operator<<(in);
      return *this;
   }


   template<typename TRow>
   cactive_criteria_t& EQ (const ccriteria_fieldnm_t<TRow>& in) 
   {
      this->operator<<(op::EQ());
      this->operator<<(in);
      return *this;
   }

   template<typename TRow>
   cactive_criteria_t& OP (const ccriteria_fieldnm_t<TRow>& in) 
   {
      this->operator<<(op::OP());
      this->operator<<(in);
      return *this;
   }

   template<typename TRow>
   cactive_criteria_t& CP (const ccriteria_fieldnm_t<TRow>& in) 
   {
      this->operator<<(op::CP());
      this->operator<<(in);
      return *this;
   }
   //END OF CRITERIA TO FIELDS logic OPS






 
   //bindable meaning that the data will be supplied
   //by the program
   cactive_criteria_t& operator<< (const ccriteria_field_t& bf)
   {
      char buf[256];//field name will fit into this buffer
      std::string tmp=bf.dbname();
      sprintf(buf,tmp.c_str(),m_str.length());
      
      this->operator<<(buf);
      
      return *this;
   }


   //Another form of bindable field. This one was created
   //by supplying row type information at compile time.
   //
   //bindable meaning that the data will be supplied
   //by the program
   template<typename TRow>
   cactive_criteria_t& operator<< (const ccriteria_fieldnm_t<TRow>& bf)
   {
      char buf[256]; //field name will fit into this buffer
      std::string tmp=bf.dbname();
      sprintf(buf,tmp.c_str(),m_str.length());
      
      
      this->operator<<(buf);
      
      return *this;
   }




   //local meaning -- local to the dabase not to the code
   cactive_criteria_t& operator<< (const ccriteria_local_field_t& lf)
   {
      m_str=m_str+lf.dbname();
      return *this;
   }

   template<typename TVal>
   cactive_criteria_t& LT(const TVal& in)
   {
      this->operator<<(op::LT());
      std::string v=boost::lexical_cast<std::string>(in);
      this->operator<<(v);
      return *this;      
   }
   
   template<typename TVal>
   cactive_criteria_t& GT(const TVal& in)
   {
      this->operator<<(op::GT());
      std::string v=boost::lexical_cast<std::string>(in);
      this->operator<<(v);
      return *this;      
   }

   
   template<typename TVal>
   cactive_criteria_t& EQ(const TVal& in)
   {
      this->operator<<(op::EQ());
      std::string v=boost::lexical_cast<std::string>(in);
      this->operator<<(v);
      return *this;      
   }


   template<typename TVal>
   cactive_criteria_t& NEQ(const TVal& in)
   {
      this->operator<<(op::NEQ());
      std::string v=boost::lexical_cast<std::string>(in);
      this->operator<<(v);
      return *this;      
   }



   template<typename TVal>
   cactive_criteria_t& AND(const TVal& in)
   {
      this->operator<<(op::AND());
      std::string v=boost::lexical_cast<std::string>(in);
      this->operator<<(v);
      return *this;      
   }


   template<typename TVal>
   cactive_criteria_t& OR(const TVal& in)
   {
      this->operator<<(op::OR());
      std::string v=boost::lexical_cast<std::string>(in);
      this->operator<<(v);
      return *this;      
   }


   template<typename TVal>
   cactive_criteria_t& OP(const TVal& in)
   {
      this->operator<<(op::OP());
      std::string v=boost::lexical_cast<std::string>(in);
      this->operator<<(v);
      return *this;      
   }



   template<typename TVal>
   cactive_criteria_t& CP(const TVal& in)
   {
      this->operator<<(op::CP());
      std::string v=boost::lexical_cast<std::string>(in);
      this->operator<<(v);
      return *this;      
   }




   template<typename TVal>
   cactive_criteria_t& operator <<(const TVal& in)
   {
      std::string v=boost::lexical_cast<std::string>(in);
      m_str=m_str+v;
      return *this;      
   }

     
   virtual const std::string get_db_str(void) const
   {
      //replace multiple spaces between words with one space
      //so first find the sequence then replace with one space
   
      std::string res=
      boost::algorithm::replace_all_regex_copy(m_str,
      boost::regex(std::string(" +")), std::string(" ")); // output should be "1 2 3"   
      return res;
   }
   
   virtual void clear(void)
   {
      m_str="";
   }
   
   private:
      //if you add another variable, change copy constructor
      std::string m_str;
};

   



///Select fields from table with active row
//Table does not need to be a single table, of course,
//just like the active row does not need to have all fields
//from one table.  Just as long as it is a 'record' set, we call it
//a table
template <typename TResultRow >
struct cactiverow_selector_t
{
   typedef  TResultRow tResultRow;

   ///This constructor will populate result 
   //datamember whith string representing ordered
   //set of fields
   cactiverow_selector_t (void)
   :m_toplimit(-1)
   {
      m_rowFields=TResultRow::get_ordered_fields_static ();
      result=result_select_str();
   };
   
   cactiverow_selector_t (const cactiverow_t::tOrderedFields& fields)
   :m_rowFields(fields), m_toplimit(-1)
   {
      result=result_select_str();
   }  
   
   virtual ~cactiverow_selector_t(void){};
   
   void limit_top(int num)
   {
      if (num>0)
      {
         m_toplimit=num;
      }   
      
   }
   
   //create the select string that is good enough
   //to be passed to otl_stream
   //if the criteria is empty, do not create the 'where'
   virtual std::string get_db_str (void) const
   {
      std::string res;
      if (criteria.get_db_str().length()<1)
      {
         res= ( "select " + result+
               +" from "+from);
      
      }
      else
      {
         res= ( "select " + result+
               +" from "+from+" where "+
               criteria.get_db_str() );
      }      
      
      if (m_toplimit>0)
      {
         //if postgresql we append to the end
         res=res+" "+"LIMIT  "+boost::lexical_cast<std::string>(m_toplimit);
      }
      return res;         
   }
   
   virtual std::string result_select_str (void) const
   {
      //notice that all the fields are sorted in the
      //order that was specified during DECL_FIELD macro
      return TResultRow::get_fieldslist_str_sqlout(m_rowFields);      
   }
   
   //unique tag representing the select
   //statement here
   std::string utag (void) const
   {
      //will cheat for the time being
      //should have a good almost unique hash here.
      return this->select_str();
   }
   
   
   ///this data member is exposed so the string
   //can be overritten with custom resultselector
   //important to understand, however, that 
   // while reading the data back, I expect
   //that otl results are ordered in the same
   //way that the row reader expects them.
   std::string result;
   ///this criteria is exposed so that it can be <<
   //into 
   cactive_criteria_t criteria;
   ///this string is exposed so that tables can be listed
   std::string from;
   private:
      typename TResultRow::tOrderedFields m_rowFields;
      ///limit the number of rows selected from the top
      int m_toplimit;
      

};//end of cactiverow_selector_t


template <typename TInsertRow >
struct cactiverow_inserter_t: public cinsert_criteria_t<TInsertRow>
{

   ///This constructor will populate result 
   //datamember whith string representing ordered
   //set of fields
   cactiverow_inserter_t (const std::string& tb)
      :into(tb)
   {
   };
   
   
   
   virtual std::string get_db_str (void) const
   {
      
        return ( "insert into "+into+"( " +this->get_fields_str() +")"
               +" values ("+this->get_val_str() +" )"
                );
                      
   }
     
  protected:    
   std::string into;

};//end of cactiverow_inserter_t


///A shortcut setups the strings to insert all fields
template <typename TInsertRow >
struct cactiverow_inserter_all_t: public cactiverow_inserter_t<TInsertRow>
{
   cactiverow_inserter_all_t (const std::string& tbinto)
      :cactiverow_inserter_t<TInsertRow>(tbinto)
   {      
      typename TInsertRow::tOrderedFields m_rowFields=TInsertRow::get_ordered_fields_static ();
      
      //comma delimited field list prepared for sql-in (not sql-out)
      //std::string fields=TResultRow::get_fieldslist_str_sqlin(m_rowFields);      
      
      typename TInsertRow::tOrderedFieldsConstIter b_it=m_rowFields.begin();      
      for (b_it=m_rowFields.begin();b_it!=m_rowFields.end();++b_it)
      {
         this->operator<<(ccriteria_fieldnm_t<TInsertRow>(b_it->fname()));
      }

   }
}; //end of cactiverow_inserter_all_t


///select all fields. this is a 'shortuct' class
//There are two constructors, one that takes a name
//of the table and that means select all the fields
//and all the rows
//and the other that takes in a table name and
//the 'where' clause (which is the criteria)
//for that constructor, the reader must be supplied
//with the data for all the 'bound' fields

   template <typename TRow>
 struct cactiverow_selector_all_t: public cactiverow_selector_t<TRow>
 {
   cactiverow_selector_all_t  (const std::string& tbname)
   {
      this->from=tbname;
   }
   
   cactiverow_selector_all_t  (const std::string& tbname,
                       const cactive_criteria_t& in_criteria )
   {
      this->from=tbname;
      this->criteria=in_criteria;
   }
 };//end of cactiverow_selector_all_t





template <typename TInsertRow >
struct cactiverow_updater_t
{

   ///This constructor will populate result 
   //datamember whith string representing ordered
   //set of fields
   cactiverow_updater_t (const std::string& tb)
      :m_tbnm(tb)
   {
   };
   
   cactive_criteria_t   setwhat;
   cactive_criteria_t   setwhere;
   
   
   virtual std::string get_db_str (void) const
   {
     return ( "update "+m_tbnm+std::string(" set ") +setwhat.get_db_str()
       + std::string(" where ") + setwhere.get_db_str());                      
   }
     
  protected:    
   std::string m_tbnm;

};//end of cactiverow_updaterr_t





template <typename TInsertRow >
struct cactiverow_deleter_t
{

   ///This constructor will populate result 
   //datamember whith string representing ordered
   //set of fields
   cactiverow_deleter_t (const std::string& tb)
      :m_tbnm(tb)
   {
   };
   
   cactive_criteria_t   delwhere;
   
   
   virtual std::string get_db_str (void) const
   {
     return ( "delete from "+m_tbnm
       + std::string(" where ") + delwhere.get_db_str());                      
   }
     
  protected:    
   std::string m_tbnm;

};//end of cactiverow_deleter_t









//class that stores reference counted pointers of instances of rows
template <typename TRow, typename TContainer=typename TRow::tThisClassMSet>
struct cactivetable_t: public TContainer,
                        private boost::noncopyable

{
  

   /// select using the fields from the result table
   //  and the criteria from the criteria   
   
   static const int otl_buff_size=50;


   typedef typename TContainer::iterator  tThisClassIterator;
   typedef typename TContainer::const_iterator  tThisClassConstIterator;

   typedef typename TContainer  tThisClassParent;
   
   cactivetable_t (void)      
   {      
   }
   


   
   void read_intothis (otl_stream& stream)
   {
      typename TRow::tThisClassSharedPtr p;     
      //tThisClassSetConstIter 
      while(!stream.eof())
      { // while not end-of-data
         p.reset(new TRow());   
         stream>>*p;
         this->insert(p);   
      }   
   } //read_intothis
   
   ///insert from this table into db
   void insert_fromthis (otl_stream& stream)
   {
      typename TRow::tThisClassSharedPtr p;     
      //begin iterator
      typename TRow::tThisClassSetConstIter b_it;
      for (b_it=this->begin();b_it!=this->end();++b_it)
      {
         stream<<*(*b_it);
      }
   
   }                        
   
     void get_selectall_stream ( const std::string& from,
                             otl_stream& stream,
                                 otl_connect& conn)
   {
      cactiverow_selector_all_t<TRow> sel(from);         
      stream.open(otl_buff_size,
              sel.get_db_str().c_str(),
              conn);                 
   }


   std::string get_selectall_str ( const std::string& from)
   {
      cactiverow_selector_all_t<TRow> sel(from);               
      return sel.select_str();
   }


   
   void  get_selectall_stream ( const std::string& from,
                                    const cactive_criteria_t& crt,
                                    otl_stream& stream,
                                 otl_connect& conn)
   {
      cactiverow_selector_all_t<TRow> sel(from,crt);   
      stream.open(otl_buff_size,
              sel.select_str().c_str(),
              conn);   
                 
   }

   
   
   //insert can modify the rows stored here (for example
   //after obtaining autgenerated ids) therefore inserts
   //cannot be 'const'
   void get_insertall_stream (const std::string& tbnm,
                                 otl_stream& stream,
                                    otl_connect& conn)
   {
   
      cactiverow_inserter_all_t<TRow> ins(tbnm);
      
       stream.open(otl_buff_size,
              ins.get_db_str().c_str(),
              conn); 
//std::cout<<"insert str: "<<ins.insert_str()<<std::endl;              
   
   }

   
   
   ///get the sequence generated id after typically an insert
   //into a given table
   static otl_value<OTL_BIGINT>
         get_last_id(const std::string& tbnm,
         
                       otl_conn& conn)
   {
     assert(0);//this is db specific and for pg I just use
     //select from sequence
   }                       

};//end of cactivetable_t


//Short form of some of the classes

//local to the db field name
#define FLDNM ccriteria_local_field_t
//bound value expression for a given field name
#define BINDNM(Row,Fnm) ccriteria_fieldnm_t<Row>(Fnm)






}; //end of namespace otlexpr1

#endif
