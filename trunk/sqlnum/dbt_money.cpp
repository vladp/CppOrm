
/* copyright Vladislav Papayan 2008,2009
   vpapayan @ g maiX.com
   replace X with l
   
   License for this file:  Public domain   
   there is no implied or expressed warranty
   by the author
      
   Implementation of cmoney_t class

 $id$
*/


#if !defined (OTLEXPR1_IN_PRECOMPILED_HEADERS)

#if defined(_WINDOWS)
  #include <windows.h>
#endif

#include <iostream>
#include <string>


#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <boost/enable_shared_from_this.hpp>

#endif



#if defined(unix) || defined(__unix__) || defined(__unix)
# define PREDEF_PLATFORM_UNIX
#endif
#if defined(PREDEF_PLATFORM_UNIX)
# include <unistd.h>
#include <wchar.h>
#include <iconv.h>
#endif


#if ! defined(MARKUP_SIZEOFWCHAR)
#if __SIZEOF_WCHAR_T__ == 4 || __WCHAR_MAX__ > 0x10000
#define MARKUP_SIZEOFWCHAR 4
#else
#define MARKUP_SIZEOFWCHAR 2
#endif
#endif





//this must be included BEFORE
//the dec stuff
#include "dbt_money.hpp"



//to support money (decimal(19,6) for example)
extern "C"
{
   #include <decNumber/decQuad.h>
   #include <decNumber/decimal64.c>
}


//typedef  decQuad cmoney_impl_dtl;
struct  cmoney_impl_dtl
{
   cmoney_impl_dtl(void){};
   cmoney_impl_dtl(const decQuad& in)
      :impl(in){}
   operator decQuad () { return impl;}
   operator const decQuad ()const {return impl;}      
      
   decQuad impl;
};
    
    
cmoney_t::cmoney_t (void)
   :v(new cmoney_impl_dtl)
{

}        
    
cmoney_t::cmoney_t (const int& in)
   :v(new cmoney_impl_dtl)
{

   decQuadFromInt32( &v->impl,in);
}    
    
    
    
    
//copy constructor    
cmoney_t::cmoney_t (const cmoney_t& in)
   :v(new cmoney_impl_dtl)
 {
   *this=in;
 }
    
cmoney_t::cmoney_t (const std::string& in)
   :v(new cmoney_impl_dtl)
{
   this->from_str(in);
}
    
#ifndef BOOST_NO_STD_WSTRING
cmoney_t::cmoney_t (const std::wstring& in)
   :v(new cmoney_impl_dtl)
{
   this->from_wstr(in);
}
#endif
    
cmoney_t::cmoney_t(const cmoney_t::t_money_storage& in)
   :v(new cmoney_impl_dtl)
{
   decQuadCopy(&v->impl, &in->impl);
}


  cmoney_t::~cmoney_t()
  {
  
      //if using regular pointer do delete
      //delete v;
      v.reset();      
  };


const cmoney_t::t_money_storage& 
cmoney_t::val() const
{
   return v;
}

/*
cmoney_t::t_money_storage& 
cmoney_t::val()
{
   return v;
}
*/

/*
cmoney_t::operator cmoney_t::t_money_storage ()
{
   return v;
}
*/


std::string 
cmoney_t::as_str (void) const
{

   char strbuff2[DECQUAD_String];         
   decQuadToEngString(&v->impl, strbuff2);
   
   return std::string(strbuff2);
}




#ifndef BOOST_NO_STD_WSTRING
std::wstring
cmoney_t::as_wstr (void) const
{

   char strbuff2[DECQUAD_String];
   decQuadToEngString(&v->impl, strbuff2);
 //this will take double the space 
 //or quadrupl the space on unix
   wchar_t wData[DECQUAD_String];

   
#ifdef _WINDOWS   
::MultiByteToWideChar(CP_UTF8, 0, strbuff2, -1, wData, DECQUAD_String);
#else
   assert(0);
   iconv_t cd;
   //cd = iconv_open ("WCHAR_T", charset);
   cd = iconv_open("WCHAR_T", "UTF-8");   

//"UTF-32" or "UTF-16"   
   
   if (cd == (iconv_t) -1)
   {
      /* Something went wrong.  */
      if (errno == EINVAL)
      {
        error (0, 0, "conversion from '%s' to wchar_t not available",
                    "UTF-8");
      }                    
      else
      {
         perror ("iconv_open");     
        /* Terminate the output string.  */
        //   *outbuf = L'\0';     
        //     return -1;
      }
    }
   
    size_t  input_size=strlen(strbuff2);
    size_t output_size=sizeof_(wchar_t)*DECQUAD_String;
    iconv(cd,&strbuff2,&input_size,&wData,&output_size);
    

#endif
return std::wstring(wData);   

}

#endif // wstring    

void  
cmoney_t::from_str (const std::string& in)
{
   decContext set;
   //char strbuff2[DECQUAD_String];   
   decContextDefault(&set, DEC_INIT_DECQUAD); // initialize4.  
   decQuadFromString( &v->impl,in.c_str(),&set);   
}
   
#ifndef BOOST_NO_STD_WSTRING
void  
cmoney_t::from_wstr (const std::wstring& in)
{

char utf8ASCIIstr[DECQUAD_String];   
   
#ifdef WIN32

/*
int WideCharToMultiByte(
  UINT CodePage, 
  DWORD dwFlags, 
  LPCWSTR lpWideCharStr, 
  int cchWideChar, 
  LPSTR lpMultiByteStr, 
  int cbMultiByte, 
  LPCSTR lpDefaultChar, 
  LPBOOL lpUsedDefaultChar 
);
*/


::WideCharToMultiByte(CP_UTF8, 0, 
                           (LPWSTR)in.c_str(), in.length(),
                           utf8ASCIIstr, DECQUAD_String,NULL,NULL);
                           
                           
   decContext set;   
   decContextDefault(&set, DEC_INIT_DECQUAD); // initialize4.  
   decQuadFromString( &v->impl,utf8ASCIIstr,&set);
                           
#else   

   assert(0);
   std::cerr<<"cannot convert to decQuad from widechar "<<endl;
#endif
   
}

#endif //wstring

        

cmoney_t& cmoney_t::operator=(const cmoney_t& in)
{

   decQuadCopy(&v->impl, &in.v->impl);
   return *this;
}

bool 
cmoney_t::operator<(const cmoney_t& in ) const
{
  decContext set;
  decContextDefault(&set, DEC_INIT_DECQUAD); // initialize4.     
  decQuad result;
  decQuadCompare (&result,&v->impl,&in.v->impl,&set);   
  
  int intresult=decQuadToInt32(&result, &set, DEC_ROUND_UP);
  
  if (intresult==-1)
  {
   return true;
  }
  else
  {
   return false;
  }
 
   
}



cmoney_t& 
cmoney_t::operator+=(const cmoney_t& in)
{   
   decContext set;   
   decContextDefault(&set, DEC_INIT_DECQUAD); // initialize4.     

   //decQuadPlus (&v->impl,&in.v->impl,&set);   
   decQuadAdd (&v->impl,&v->impl,&in.v->impl,&set);   
   return *this;  
}


cmoney_t& 
cmoney_t::operator+=(const int& in)
{
   this->operator+=(cmoney_t(in));
   return *this;
}


cmoney_t& 
cmoney_t::operator-=(const cmoney_t& in)
{
   decContext set;
   decContextDefault(&set, DEC_INIT_DECQUAD); // initialize4.     
   decQuadSubtract (&v->impl,&v->impl,&in.v->impl,&set);   
   return *this;  
}


cmoney_t& 
cmoney_t::operator-=(const int& in)
{
   this->operator-=(cmoney_t(in));
   return *this;
}

 
cmoney_t& 
cmoney_t::operator*=(const cmoney_t& in)
{
  decContext set;
  decContextDefault(&set, DEC_INIT_DECQUAD); // initialize4.     
  decQuadMultiply (&v->impl,&v->impl,&in.v->impl,&set);   
  return *this;  
}


cmoney_t& 
cmoney_t::operator*=(const int& in)
{
   this->operator*=(cmoney_t(in));
   return *this;
}


cmoney_t& 
cmoney_t::operator/=(const cmoney_t& in)
{
  decContext set;
  decContextDefault(&set, DEC_INIT_DECQUAD); // initialize4.     
  decQuadDivide (&v->impl,&v->impl,&in.v->impl,&set);   
  return *this;  

}



cmoney_t& 
cmoney_t::operator/=(const int& in)
{
   this->operator/=(cmoney_t(in));
   return *this;
}

