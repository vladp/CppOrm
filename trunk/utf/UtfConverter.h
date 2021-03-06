#ifndef UTFCONVERTER__H__
#define UTFCONVERTER__H__

#include "ConvertUTF.h"

#if defined (__CYGWIN__)
/* i am not including boost here, therefore if was not included before mimic its macro
  that shows that wstring is not defined
*/
#ifndef  BOOST_NO_STD_WSTRING
#define  BOOST_NO_STD_WSTRING
#endif
#endif







// http://www.codeproject.com/KB/string/UtfConverter.aspx
namespace UtfConverter
{





#ifndef BOOST_NO_STD_WSTRING
    std::wstring FromUtf8(const std::string& utf8string);
    std::string ToUtf8(const std::wstring& widestring);



   inline
	std::wstring FromUtf8(const std::string& utf8string)
	{
		size_t widesize = utf8string.length();
		if (sizeof(wchar_t) == 2)
		{
			std::wstring resultstring;
			resultstring.resize(widesize+1, L'\0');
			const UTF8* sourcestart = reinterpret_cast<const UTF8*>(utf8string.c_str());
			const UTF8* sourceend = sourcestart + widesize;
			UTF16* targetstart = reinterpret_cast<UTF16*>(&resultstring[0]);
			UTF16* targetend = targetstart + widesize;
			ConversionResult res = ConvertUTF8toUTF16(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
			if (res != conversionOK)
			{
			  throw std::runtime_error(std::string("La falla!"));
			}
			*targetstart = 0;
			return resultstring;
		}
		else if (sizeof(wchar_t) == 4)
		{
			std::wstring resultstring;
			resultstring.resize(widesize+1, L'\0');
			const UTF8* sourcestart = reinterpret_cast<const UTF8*>(utf8string.c_str());
			const UTF8* sourceend = sourcestart + widesize;
			UTF32* targetstart = reinterpret_cast<UTF32*>(&resultstring[0]);
			UTF32* targetend = targetstart + widesize;
			ConversionResult res = ConvertUTF8toUTF32(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
			if (res != conversionOK)
			{
				throw std::runtime_error("La falla!");
			}
			*targetstart = 0;
			return resultstring;
		}
		else
		{
			throw std::runtime_error("La falla!");
		}
		return L"";
	}

   inline
	std::string ToUtf8(const std::wstring& widestring)
	{
		size_t widesize = widestring.length();

		if (sizeof(wchar_t) == 2)
		{
			size_t utf8size = 3 * widesize + 1;
			std::string resultstring;
			resultstring.resize(utf8size, '\0');
			const UTF16* sourcestart = reinterpret_cast<const UTF16*>(widestring.c_str());
			const UTF16* sourceend = sourcestart + widesize;
			UTF8* targetstart = reinterpret_cast<UTF8*>(&resultstring[0]);
			UTF8* targetend = targetstart + utf8size;
			ConversionResult res = ConvertUTF16toUTF8(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
			if (res != conversionOK)
			{
				throw std::runtime_error("La falla!");
			}
			*targetstart = 0;
			return resultstring;
		}
		else if (sizeof(wchar_t) == 4)
		{
			size_t utf8size = 4 * widesize + 1;
			std::string resultstring;
			resultstring.resize(utf8size, '\0');
			const UTF32* sourcestart = reinterpret_cast<const UTF32*>(widestring.c_str());
			const UTF32* sourceend = sourcestart + widesize;
			UTF8* targetstart = reinterpret_cast<UTF8*>(&resultstring[0]);
			UTF8* targetend = targetstart + utf8size;
			ConversionResult res = ConvertUTF32toUTF8(&sourcestart, sourceend, &targetstart, targetend, strictConversion);
			if (res != conversionOK)
			{
				throw std::runtime_error("toUtf8 failed!");
			}
			*targetstart = 0;
			return resultstring;
		}
		else
		{
			throw std::runtime_error("toUtf8 failed!");
		}
		return "";
	}

#endif //BOOST_NO_STD_WSTRING
};



#endif
