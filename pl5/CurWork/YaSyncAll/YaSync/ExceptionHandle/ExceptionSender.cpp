/* zpipe.c: example of proper use of zlib's inflate() and deflate()
Not copyrighted -- provided to the public domain
Version 1.4  11 December 2005  Mark Adler */

/* Version history:
1.0  30 Oct 2004  First version
1.1   8 Nov 2004  Add void casting for unused return values
Use switch statement for inflate() return values
1.2   9 Nov 2004  Add assertions to document zlib guarantees
1.3   6 Apr 2005  Remove incorrect assertion in inf()
1.4  11 Dec 2005  Add hack to avoid MSDOS end-of-line conversions
Avoid some compiler warnings for input and output buffers
*/
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "zlib.h"
#include "afxinet.h"
#include "ExceptionHandler.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

/* Compress from file source to file dest until EOF on source.
def() returns Z_OK on success, Z_MEM_ERROR if memory could not be
allocated for processing, Z_STREAM_ERROR if an invalid compression
level is supplied, Z_VERSION_ERROR if the version of zlib.h and the
version of the library linked do not match, or Z_ERRNO if there is
an error reading or writing the files. */
int def(FILE *source, FILE *dest, int level)
{
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION,Z_DEFLATED, MAX_WBITS+16,8, Z_DEFAULT_STRATEGY);
	if (ret != Z_OK)
		return ret;

	/* compress until end of file */
	do {
		strm.avail_in = (uInt)fread(in, 1, CHUNK, source);
		if (ferror(source)) {
			(void)deflateEnd(&strm);
			return Z_ERRNO;
		}
		flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;

		/* run deflate() on input until output buffer not full, finish
		compression if all of source has been read in */
		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = deflate(&strm, flush);    /* no bad return value */
			assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			have = CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
		assert(strm.avail_in == 0);     /* all input will be used */

		/* done when last data in file processed */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END);        /* stream will be complete */

	/* clean up and return */
	(void)deflateEnd(&strm);
	return Z_OK;
}


/* report a zlib or i/o error */
void zerr(int ret)
{
	fputs("zpipe: ", stderr);
	switch (ret) {
	case Z_ERRNO:
		if (ferror(stdin))
			fputs("error reading stdin\n", stderr);
		if (ferror(stdout))
			fputs("error writing stdout\n", stderr);
		break;
	case Z_STREAM_ERROR:
		fputs("invalid compression level\n", stderr);
		break;
	case Z_DATA_ERROR:
		fputs("invalid or incomplete deflate data\n", stderr);
		break;
	case Z_MEM_ERROR:
		fputs("out of memory\n", stderr);
		break;
	case Z_VERSION_ERROR:
		fputs("zlib version mismatch!\n", stderr);
	}
}

#include "RyeolHttpClient.h"
#include "RyeolException.h"
using namespace Ryeol;

void SendSupportReq(TCHAR *szEmail,TCHAR *szProblem)
{
	CHttpClient         objHttpReq ;
	CHttpResponse *     pobjHttpRes = NULL ;

	try {
		// Initialize the User Agent
		objHttpReq.SetInternet (_T ("YaSync CR")) ;

		// Add user's custom HTTP headers
		/*objHttpReq.AddHeader (_T ("Ryeol-Magic"), _T ("My Magic Header")) ;
		objHttpReq.AddHeader (_T ("User-Magic"), _T ("User's Magic Header")) ;*/

		// Add user's parameters
		objHttpReq.AddParam (_T ("EMAIL"), szEmail);
		objHttpReq.AddParam (_T ("PROBLEM"), szProblem);
		

		// Start a new request
		objHttpReq.BeginPost (_T ("http://a.mobitnt.com/supportReq.php")) ;

		// Specifies the number of bytes to send when the Proceed method is called.
		const DWORD     cbProceed = 1024 ;  // 1K

		do {
			int i = 0;

		} while ( !(pobjHttpRes = objHttpReq.Proceed (cbProceed)) ) ;

	} catch (httpclientexception e) {

	}
}

void SendFile(TCHAR *szFilePath,TCHAR *szFileName)
{
	CHttpClient         objHttpReq ;
	CHttpResponse *     pobjHttpRes = NULL ;

	try {
		// Initialize the User Agent
		objHttpReq.SetInternet (_T ("YaSync CR")) ;

		// Add user's custom HTTP headers
		/*objHttpReq.AddHeader (_T ("Ryeol-Magic"), _T ("My Magic Header")) ;
		objHttpReq.AddHeader (_T ("User-Magic"), _T ("User's Magic Header")) ;*/

		// Add user's parameters
		objHttpReq.AddParam (_T ("FileName"), szFileName) ;
		
		// Specifies a file to upload
		objHttpReq.AddParam (szFileName,szFilePath, CHttpClient::ParamFile);

		// Start a new request
		objHttpReq.BeginUpload (_T ("http://a.mobitnt.com/cr.php")) ;

		// Specifies the number of bytes to send when the Proceed method is called.
		const DWORD     cbProceed = 1024 ;  // 1K

		do {
			int i = 0;

		} while ( !(pobjHttpRes = objHttpReq.Proceed (cbProceed)) ) ;

	} catch (httpclientexception e) {

	}
} 

#ifndef PE_LOG_FILE_NAME
#define		PE_LOG_FILE_NAME									_T("YaSync_Log.txt")
#endif

extern int GetAppDataPath(TCHAR* szPath);
void SendLogFile()
{
	TCHAR szDataPath[MAX_PATH];
	GetAppDataPath(szDataPath);
	TCHAR szZipPath[MAX_PATH];

	TCHAR szFile[500];
	_stprintf(szFile,_T("%s\\%s"),szDataPath,XCRASHREPORT_ERROR_LOG_FILE);
	
	FILE *fExceptionFile = _tfopen(szFile,_T("rb"));
	if (fExceptionFile)
	{
		_stprintf(szZipPath,_T("%s\\Exception.gz"),szDataPath);
		FILE *fZipFile = _tfopen(szZipPath,_T("wb+"));
		if (fZipFile)
		{
			if (Z_OK == def(fExceptionFile,fZipFile,1))
			{
				fclose(fZipFile);
				SendFile(szZipPath,_T("Exceptiongz"));
			}
			else
			{
				fclose(fZipFile);
			}

			_tremove(szZipPath);
		}

		fclose(fExceptionFile);
		_tremove(szFile);
	}

	PEWriteLog(_T("Send Exception done"));

	_stprintf(szFile,_T("%s\\%s"),szDataPath,PE_LOG_FILE_NAME);
	FILE *fLogFile = _tfopen(szFile,_T("rb"));
	if (fLogFile)
	{
		_stprintf(szZipPath,_T("%s\\Log.gz"),szDataPath);
		FILE *fZipFile = _tfopen(szZipPath,_T("wb+"));
		if (fZipFile)
		{
			if (Z_OK == def(fLogFile,fZipFile,Z_DEFAULT_COMPRESSION))
			{
				fclose(fZipFile);
				SendFile(szZipPath,_T("Loggz"));
			}
			else
			{
				fclose(fZipFile);
			}
			_tremove(szZipPath);
		}

		fclose(fLogFile);
		_tremove(szFile);
	}
	
	PEWriteLog(_T("Send Log done"));

	_stprintf(szFile,_T("%s\\%s"),szDataPath,PE_LOG_FILE_NAME);
	extern char g_szLogFilePathA[MAX_PATH];
	fLogFile = fopen(g_szLogFilePathA,"rb");
	if (fLogFile)
	{
		_stprintf(szZipPath,_T("%s\\LogA.gz"),szDataPath);
		FILE *fZipFile = _tfopen(szZipPath,_T("wb+"));
		if (fZipFile)
		{
			if (Z_OK == def(fLogFile,fZipFile,Z_DEFAULT_COMPRESSION))
			{
				fclose(fZipFile);
				SendFile(szZipPath,_T("LogAgz"));
			}
			else
			{
				fclose(fZipFile);
			}
			_tremove(szZipPath);
		}

		fclose(fLogFile);
		_tremove(szFile);
	}

	PEWriteLog(_T("Send Log A done"));
	
	_stprintf(szFile,_T("%s\\%s"),szDataPath,XCRASHREPORT_MINI_DUMP_FILE);
	FILE *fDumpFile = _tfopen(szFile,_T("rb"));
	if (fDumpFile)
	{
		_stprintf(szZipPath,_T("%s\\Dump.gz"),szDataPath);
		FILE *fZipFile = _tfopen(szZipPath,_T("wb+"));
		if (fZipFile)
		{
			if (Z_OK == def(fDumpFile,fZipFile,Z_DEFAULT_COMPRESSION))
			{
				fclose(fZipFile);
				SendFile(szZipPath,_T("Dumpgz"));
			}
			else
			{
				fclose(fZipFile);
			}
			_tremove(szZipPath);
		}

		fclose(fDumpFile);
		_tremove(szFile);
	}

	PEWriteLog(_T("Send DMP done"));
}