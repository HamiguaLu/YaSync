#include "stdafx.h"

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include "UrlEscape.h"


#if 0

#define IsHexNum(c) ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))

CString Utf8ToStringT(LPSTR str)
{
    _ASSERT(str);
    USES_CONVERSION;
    WCHAR *buf;
    int length = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    buf = new WCHAR[length+1];
    ZeroMemory(buf, (length+1) * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, buf, length);

    return (CString(W2T(buf)));
}


CString UrlDecode(LPCTSTR url)
{
    _ASSERT(url);
    USES_CONVERSION;
    LPSTR _url = T2A(const_cast<LPTSTR>(url));
    int i = 0;
    int length = (int)strlen(_url);
    CHAR *buf = new CHAR[length];
    ZeroMemory(buf, length);
    LPSTR p = buf;
    char tmp[4];
    while(i < length)
    {
        if(i <= length -3 && _url[i] == '%' && IsHexNum(_url[i+1]) && IsHexNum(_url[i+2]))
        {
            memset(tmp, 0, sizeof(tmp));
            memcpy(tmp, _url + i + 1,2); 
            sscanf(tmp, "%x", p++);
            i += 3;
        }
        else
        {
            *(p++) = _url[i++];
        }
    }
    return Utf8ToStringT(buf);
}



#include <windows.h>
#include <string>
#include <shlwapi.h>
#include <sstream>
#include <iostream>
#include <wininet.h> // For INTERNET_MAX_URL_LENGTH

#pragma comment(lib, "Shlwapi.lib")

bool IsHexChar(const WCHAR _char)
{
    return ((_char == L'A') || 
            (_char == L'B') ||
            (_char == L'C') ||
            (_char == L'D') ||
            (_char == L'E') ||
            (_char == L'F') || 
            iswalnum(_char));
}

std::wstring UrlDecode(const std::wstring& _encodedStr)
{
    std::string charStr;

    for (size_t i = 0; i < _encodedStr.length(); ++i)
    {
        if ((_encodedStr[i] == L'%') && (IsHexChar(_encodedStr[i+1])) && (IsHexChar(_encodedStr[i+2])))
        {
            std::wstring hexCodeStr = L"0x";
            hexCodeStr += _encodedStr[i+1];
            hexCodeStr += _encodedStr[i+2];

            unsigned int hexCharCode;   
            std::wstringstream ss;
            ss << std::hex << hexCodeStr;
            ss >> hexCharCode;

            charStr += static_cast<char>(hexCharCode);

            i += 2;
        }
        else if (_encodedStr[i] == L'+')
            charStr += L' ';
        else
            charStr += _encodedStr[i];
    }

    WCHAR decodedStr[INTERNET_MAX_URL_LENGTH];
    MultiByteToWideChar(CP_UTF8, 0, charStr.c_str(), -1, decodedStr, sizeof(decodedStr));

    return decodedStr;
}



int URLDecoder1(wchar_t *pSrc,wchar_t *pOut,int iOutBufLen)
{
	std::wstring encodeStr = pSrc;
	std::wstring sOut = UrlDecode(encodeStr);
	if (iOutBufLen <= sOut.length())
	{
		return -1;
	}

	_tcscpy(pOut,sOut.c_str());
	return 0;
	
}

int URLDecoder2(wchar_t *pSrc,wchar_t *pOut,int iOutBufLen)
{
	CString sOut = UrlDecode(pSrc);
	if (iOutBufLen <= sOut.GetLength())
	{
		return -1;
	}

	_tcscpy(pOut,sOut.GetBuffer());
	sOut.ReleaseBuffer();

	return 0;

}

#endif


int URLDecoder(wchar_t *pSrc,wchar_t *pOut,int iOutBufLen)
{
	int iSrcLen = (int)_tcslen(pSrc);
	if (iSrcLen < 1 || iOutBufLen < iSrcLen)
	{
		return -1;
	}
	
	int iSrcPos = 0; 
	int iDstPos = 0;
	char *pDstStr = new char[iSrcLen + 1];
	memset(pDstStr,0,iSrcLen + 1);

	while (iSrcPos < iSrcLen)
	{
		char ch = (char)pSrc[iSrcPos];
		if (ch == '+')
		{
			pDstStr[iDstPos] = ' ';
		}
		else if (ch == '%')
		{
			if (iSrcPos + 2 >= iSrcLen)
			{
				break;
			}
			wchar_t hex[3];
			hex[0] = pSrc[iSrcPos + 1]; 
			hex[1] = pSrc[iSrcPos + 2];
			hex[2] = 0;
			
			pDstStr[iDstPos] = (char)wcstol(hex,NULL,16);;
			iSrcPos += 2;
		}
		else
		{
			// 取出不必转换的字符
			pDstStr[iDstPos] = ch;
		}

		++iSrcPos;
		++iDstPos;
	}

	MultiByteToWideChar(CP_UTF8,0,pDstStr,-1,pOut,iOutBufLen);
	delete[] pDstStr;

	return 0;
}


static unsigned char hexchars[] = "0123456789ABCDEF";

static int php_htoi(char *s)
{
	int value;
	int c;

	c = ((unsigned char *)s)[0];
	if (isupper(c))
		c = tolower(c);
	value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

	c = ((unsigned char *)s)[1];
	if (isupper(c))
		c = tolower(c);
	value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

	return (value);
}

TCHAR *URLEncodeW(TCHAR *s, int len)
{
	int iLen = WideCharToMultiByte(CP_UTF8,0,s,(int)_tcslen(s),0,0,0,0);
	char *szData = (char *)calloc(1,iLen + 1);
	WideCharToMultiByte(CP_UTF8,0,s,(int)_tcslen(s),szData,iLen,0,0);

	char *sDst = URLEncodeA(szData,(int)strlen(szData),&iLen);
	free(szData);

	iLen = MultiByteToWideChar(CP_UTF8,0,sDst,-1,0,0);
	TCHAR *tszDst = (TCHAR*)calloc(1,iLen * sizeof(TCHAR) + 1);
	MultiByteToWideChar(CP_UTF8,0,sDst,-1,tszDst,iLen);
	
	free(sDst);

	return tszDst;
}


char *URLEncodeA(char *s, int len, int *new_length)
{
	unsigned char c;
	unsigned char *to, *start;
	unsigned char *from, *end;
	
	from = (unsigned char *)s;
	end  = (unsigned char *)s + len;
	start = to = (unsigned char *)calloc(1, 3*len+1);

	while (from < end) 
	{
		c = *from++;

		if (c == ' ') 
		{
			*to++ = '+';
		} 
		else if ((c < '0' && c != '-' && c != '.') ||
				 (c < 'A' && c > '9') ||
				 (c > 'Z' && c < 'a' && c != '_') ||
				 (c > 'z')) 
		{
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
		}
		else 
		{
			*to++ = c;
		}
	}
	*to = 0;
	if (new_length) 
	{
		*new_length = (int)(to - start);
	}
	return (char *) start;
}


int URLDecoder(char *str)
{
	char *dest = str;
	char *data = str;
	int len = (int)strlen(str);
	while (len--) 
	{
		if (*data == '+') 
		{
			*dest = ' ';
		}
		else if (*data == '%' && len >= 2 && isxdigit((int) *(data + 1)) && isxdigit((int) *(data + 2))) 
		{
			*dest = (char) php_htoi(data + 1);
			data += 2;
			len -= 2;
		} 
		else 
		{
			*dest = *data;
		}
		data++;
		dest++;
	}
	*dest = 0;
	return (int)(dest - str);
}






