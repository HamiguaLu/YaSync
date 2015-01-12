// ClientSock.cpp : implementation file
//

#include "stdafx.h"
#include "ClientSock.h"
#include "AppTools.h"

#include "UrlEscape.h"
#include "SettingHelper.h"

#include "resource.h"

#include "PECore.h"

#define BUFSIZE    8096
#define VERSION 23
#define NETBUFSIZE 8096
#define HTTPERROR      42
#define LOG        44
#define FORBIDDEN 403
#define NOTFOUND  404

#define write(x,y,z) send(x,y,z,0)
#define read(x,y,z) recv(x,y,z,0)

typedef struct {char *ext;char *filetype;} extInfo;
extInfo extensions [] = {
	{"gif", "image/gif" },  
	{"jpg", "image/jpg" }, 
	{"jpeg","image/jpeg"},
	{"png", "image/png" },  
	{"ico", "image/ico" },  
	{"zip", "image/zip" },  
	{"gz",  "image/gz"  },  
	{"tar", "image/tar" },  
	{"htm", "text/html" },  
	{"html","text/html" },
	{"js", "application/JS"},
	{"css", "text/css"},
	{"xml", "text/xml"},
	{0,0}
};

char *sz403Html = "<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n";
char *sz404Html = "<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n";

extern void PEWriteLog2FileA(char *szLogInfo);
extern DWORD WINAPI TestConnT(LPVOID  lparam);
extern void GetRestoreFolder(TCHAR *szEntryID,TCHAR *szPath);

void CClientSock::SendErrorResponse(int iCode,char *szData)
{
	char szHeader[255];
	char *sDescription = "Unkown Error";

	if (iCode == 404)
	{
		sDescription = "Not Found";
	}
	else if (iCode == 403)
	{
		sDescription = "Forbidden";
	}

	int iLen = (int)strlen(szData);
	sprintf(szHeader,
		"HTTP/1.0 %d %s\r\nServer: MiniHttpd\r\nContent-Length: %ld\r\nConnection: close\r\nContent-Type: text/html\r\n\r\n",
		iCode,sDescription,iLen);

	Send(szHeader,(int)strlen(szHeader),0);
	Send(szData,iLen,0);
}

void CClientSock::SendResponseHeader(int iLen,char *sType)
{
	char szHeader[255];
	sprintf(szHeader,
		"HTTP/1.0 200 OK\r\nServer: MiniHttpd\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n",
		iLen,sType);

	Send(szHeader,(int)strlen(szHeader),0);
}

void CClientSock::ServeRequest()
{
	int i,j, buflen,iLen,ret;

	static char buffer[NETBUFSIZE+1]; /* static so zero filled */

	ret = Receive(buffer,NETBUFSIZE); 	/* read Web request in one go */
	if(ret == 0 || ret == -1) 
	{
		/* read failure stop now */
		SendErrorResponse(403,sz403Html);
		PEWriteLog2FileA("failed to read browser request");
		//Sleep(1000);
		return;
	}

	if(ret > 0 && ret < NETBUFSIZE)
	{
		/* return code is valid chars */
		buffer[ret] = 0;		/* terminate the buffer */
	}
	else
	{
		buffer[0] = 0;
	}

	for(i=0;i<ret;i++)
	{
		/* remove CF and LF characters */
		if(buffer[i] == '\r' || buffer[i] == '\n')
		{
			buffer[i] = '*';
		}
	}

	if( strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) ) 
	{
		SendErrorResponse(403,sz403Html);
		PEWriteLog2FileA("Only simple GET operation supported");
		return;
	}

	for( i = 4; i < BUFSIZE; i++)
	{ 
		/* null terminate after the second space to ignore extra stuff */
		if(buffer[i] == ' ') 
		{ 
			/* string is "GET URL " +lots of other stuff */
			buffer[i] = 0;
			break;
		}
	}

	for(j=0;j<i-1;j++) 	/* check for illegal parent directory use .. */
	{
		if(buffer[j] == '.' && buffer[j+1] == '.')
		{
			SendErrorResponse(403,sz403Html);
			PEWriteLog2FileA("Parent directory (..) path names not supported");
			return;
		}
	}

	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) /* convert no filename to index file */
	{
		strcpy(buffer,"GET /index.html");
	}

	/* work out the file type and check we support it */


	char *szParameter = strstr(buffer,"?");
	if (szParameter)
	{
		if (strlen(szParameter) > 1)
		{
			*szParameter = 0;
			++szParameter;
		}
	}

	buflen = (int)strlen(buffer);
	char *szContentType = NULL;

	for(i=0;extensions[i].ext != 0;i++)
	{
		iLen = (int)strlen(extensions[i].ext);
		char *szExt = &buffer[buflen - iLen]; 
		if( !strncmp(szExt,extensions[i].ext, iLen))
		{
			szContentType =extensions[i].filetype;
			break;
		}
	}

	if(szContentType == 0)
	{
		SendErrorResponse(403,sz403Html);
		PEWriteLog2FileA("file extension type not supported");
		return;
	}

	char *szFileName = &buffer[5];
	if (OnAjaxRequest(szFileName,szParameter,szContentType))
	{
		return;
	}

	HRSRC hRsrc = 0;
	if (strstr(szFileName,"ThreadList.html"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_HTML_THREAD),RT_HTML);
	}
	else if (strstr(szFileName,"style.css"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_CSS_STYLE),_T("CSS"));
	}
	else if (strstr(szFileName,"jslib.js"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_JS_MERGE),_T("JS"));
	}
	else if (strstr(szFileName,"smsHelper.js"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_JS_SMS_THREAD),_T("JS"));
	}
	else if (strstr(szFileName,"contact.png"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_HTML_IMG_CONTACT),_T("HTML_IMG"));
	}
	else if (strstr(szFileName,"contactsent.png"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_HTML_IMG_CONTACT_SENT),_T("HTML_IMG"));
	}
	else if (strstr(szFileName,"time.png"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_HTML_IMG_TIME),_T("HTML_IMG"));
	}
	else if (strstr(szFileName,"timesent.png"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_HTML_IMG_TIME_SENT),_T("HTML_IMG"));
	}
	else if (strstr(szFileName,"smsloading.gif"))
	{
		hRsrc = ::FindResource (NULL,MAKEINTRESOURCE(IDR_HTML_IMG_LOADING),_T("HTML_IMG"));
	}
	
	if(hRsrc == NULL)
	{
		SendErrorResponse(404,sz404Html);
		return;
	}

	DWORD dwLen = SizeofResource(NULL, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(NULL, hRsrc);
	if (!lpRsrc)
	{
		SendErrorResponse(404,sz404Html);
		return;
	}

	SendResponseHeader(dwLen,szContentType);
	Send(lpRsrc,dwLen,0);

	FreeResource(lpRsrc);

	return;
}

char *CClientSock::GetParameterVal(char *szParameterList,char *szPName,char *sDefVal)
{
	char szTag[255];
	sprintf(szTag,"%s=",szPName);
	char *sPara = strstr(szParameterList,szTag);
	if (sPara == NULL)
	{
		return sDefVal;
	}

	char *sNextP = strstr(sPara,"&");
	char *sValStart = strstr(sPara,"=");
	if (sValStart == NULL || strlen(sValStart) < 2)
	{
		//Error
		return sDefVal;
	}

	++sValStart;
	if (sNextP == NULL)
	{
		//last Parameter
		return sValStart;
	}

	static char szVal[255];
	memset(szVal,0,255);
	strncpy(szVal,sValStart,sNextP - sValStart);

	return szVal;
}


void CClientSock::SendRetXml(char *szMsg)
{
	char s[255];
	sprintf(s,"<ret>%s</ret>",szMsg);
	int iLen = (int)strlen(s);
	SendResponseHeader(iLen,"text/xml");
	Send(s,iLen);
}

void CClientSock::SendRetXml(char *szMsg,char *szTag)
{
	char s[1024];
	sprintf(s,"<%s>%s</%s>",szTag,szMsg,szTag);
	int iLen = (int)strlen(s);
	SendResponseHeader(iLen,"text/xml");
	Send(s,iLen);
}

void CClientSock::SendRawStr(char *szMsg)
{
	Send(szMsg,(int)strlen(szMsg));
}

static TCHAR szThreadEntryID[255];
void SetThreadEntryID(TCHAR *szID)
{
	_tcscpy(szThreadEntryID,szID);
}
int CClientSock::OnAjaxRequest(char * szReqest, char *szParameter,char *szContentType)
{
	if (szParameter == NULL)
	{
		return 0;
	}

	char *szAction = GetParameterVal(szParameter,"action","");
	if (strlen(szAction) < 1)
	{
		return 0;
	}

	if (strcmp(szAction,"getchatlist") == 0)
	{
		PECore::SendMessage(WM_SHOW_THREAD,(WPARAM)szThreadEntryID);

		TCHAR *pszContent = GetThreadData();
		if (pszContent == NULL)
		{
			char *szErr = "<span>No sms found in this thread</span>";
			SendResponseHeader((int)strlen(szErr),"text/html");
			Send(szErr,(int)strlen(szErr));
			return 1;
		}

		int iLen = WideCharToMultiByte(CP_UTF8,0,pszContent,(int)_tcslen(pszContent),0,0,0,0);
		char *szData = (char *)malloc(iLen + 1);
		szData[iLen] = 0;
		WideCharToMultiByte(CP_UTF8,0,pszContent,(int)_tcslen(pszContent),szData,iLen,0,0);

		SendResponseHeader(iLen,"text/html");
		Send(szData,iLen);

		free(szData);
		delete[] pszContent;

		return 1;
	}

	if (strcmp(szAction,"deletesms") == 0)
	{
		char szThreadID[255];
		strcpy(szThreadID, GetParameterVal(szParameter,"threadid","0"));

		char szSmsID[255];
		strcpy(szSmsID, GetParameterVal(szParameter,"id","0"));

		TCHAR *tszTID = (TCHAR *)calloc(255,sizeof(TCHAR));
		TCHAR *tszMID = (TCHAR *)calloc(255,sizeof(TCHAR));

		MultiByteToWideChar(CP_ACP,0,szThreadID,-1,tszTID,255);
		MultiByteToWideChar(CP_ACP,0,szSmsID,-1,tszMID,255);


		MSG_Data_Truck *tmpTruct = (MSG_Data_Truck *)calloc(1,sizeof(MSG_Data_Truck));
		tmpTruct->p1 = (WPARAM)szThreadID;
		tmpTruct->p2 = (WPARAM)szSmsID;

		PECore::PostMessage(WM_DELETE_SMS,(WPARAM)tmpTruct,0);

		return 1;
	}


	return 0;
}

CClientSock::CClientSock()
{
}

CClientSock::~CClientSock()
{
}


// CClientSock member functions

void CClientSock::OnReceive(int nErrorCode)
{
	// TODO: Add your specialized code here and/or call the base class
	ServeRequest();

	CSocket::OnReceive(nErrorCode);

	Close();

	delete this;
}
