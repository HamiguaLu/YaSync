
#ifndef __URL_ESCAPE_HEADER__
#define __URL_ESCAPE_HEADER__


int URLDecoder(char *str);
int URLDecoder(wchar_t *pSrc,wchar_t *pOut,int iOutBufLen);

char *URLEncodeA(char *s, int len, int *new_length);
TCHAR *URLEncodeW(TCHAR *s, int len);


#endif
