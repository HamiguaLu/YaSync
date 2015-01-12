#pragma once

#include "peprotocol.h"
#include "atlstr.h"


class DeviceBlockHelper
{
public:
	static void Init();
	static void DeInit();
	static list<TCHAR *> GetFilterList();
	static int UpdateFilterList(list<TCHAR *> filterList);
	static int SaveFilter();
	static int AddFilter(TCHAR* szFilter);
	static int IsItemInFilterList(TCHAR *);
	static int FreeFilterList();
private:
	static list<TCHAR *> m_filterList;
	static 	int DeviceBlockHelper::InitFilterList();
	static void GetFilterPath(TCHAR *szPath);
};
