#include "stdafx.h"
#include "DeviceBlockHelper.h"
#include "PEProtocol.h"
#include "AppTools.h"

//TCHAR *szEndTag = _T("\r\n");

void DeviceBlockHelper::GetFilterPath(TCHAR *szPath)
{
	TCHAR szAppPath[255];
	GetAppDataPath(szAppPath);

	_stprintf(szPath,_T("%s%s"),szAppPath,PE_DEV_BLOCK_FILE_NAME);
}

list<TCHAR *> DeviceBlockHelper::m_filterList;


void DeviceBlockHelper::Init()
{
	InitFilterList();
}

void DeviceBlockHelper::DeInit()
{
	FreeFilterList();
}

int DeviceBlockHelper::FreeFilterList()
{
	list<TCHAR*>::iterator it;
	for (it = m_filterList.begin(); it != m_filterList.end(); ++it)
	{ 
		TCHAR *szFilter = (TCHAR*)*it;
		
		free(szFilter);
	}

	m_filterList.clear();

	return PE_RET_OK;
}

list<TCHAR *> DeviceBlockHelper::GetFilterList()
{
	return m_filterList;
}


int DeviceBlockHelper::InitFilterList()
{
	FreeFilterList();

	TCHAR szFilterPath[255];
	GetFilterPath(szFilterPath);
	
	FILE *pFilterFile = _tfopen(szFilterPath,_T("r"));	
	if (pFilterFile == NULL)
	{
		PEWriteLog(_T("Can't open Filter file"));
		return PE_RET_FAIL;
	}

	TCHAR szBuffer[255];
	while(1 == fread(szBuffer,255*sizeof(TCHAR),1,pFilterFile))
	{
		int iLen = (int)_tcslen(szBuffer);
		TCHAR *szFilter = (TCHAR*)calloc(iLen + 1,sizeof(TCHAR));
		_tcscpy(szFilter,szBuffer);
		m_filterList.push_back(szFilter);
	}

	fclose(pFilterFile);

	return PE_RET_OK;
}

int DeviceBlockHelper::UpdateFilterList(list<TCHAR *> filterList)
{
	FreeFilterList();

	m_filterList = filterList;

	return PE_RET_OK;
}

int DeviceBlockHelper::SaveFilter()
{
	TCHAR szFilterPath[255];
	GetFilterPath(szFilterPath);
	FILE *pFilterFile = _tfopen(szFilterPath,_T("w+b"));	
	if (pFilterFile == NULL)
	{
		PEWriteLog(_T("Can't open Filter file"));
		return 0;
	}
	
	list<TCHAR*>::iterator it;
	for (it = m_filterList.begin(); it != m_filterList.end();++it )
	{ 
		TCHAR *szFilter = (TCHAR*)*it;
		TCHAR s[255];
		memset(s,0,255*sizeof(TCHAR));
		_tcscpy(s,szFilter);
		fwrite(s,255*sizeof(TCHAR),1,pFilterFile);
	}

	fflush(pFilterFile);
	fclose(pFilterFile);

	return PE_RET_OK;
}


int DeviceBlockHelper::AddFilter(TCHAR* szFilter)
{
	if (!szFilter || _tcslen(szFilter) < 1)
	{
		return PE_RET_OK;
	}

	int iLen = (int)_tcslen(szFilter);
	TCHAR *szTmp = (TCHAR*)calloc(iLen + 1,sizeof(TCHAR));
	_tcscpy(szTmp,szFilter);
	m_filterList.push_back(szTmp);

	return PE_RET_OK;
}

int DeviceBlockHelper::IsItemInFilterList(TCHAR *contact)
{
	list<TCHAR*>::iterator it;

	for (it = m_filterList.begin(); it != m_filterList.end();++it )
	{ 
		TCHAR *szFilter = (TCHAR*)*it;
		if (_tcsstr(szFilter,contact))
		{
			return PE_RET_OK;
		}
	}

	return PE_RET_FAIL;
}

