#pragma once

#include "peprotocol.h"
#include "atlstr.h"


class FilterHelper
{
public:
	static void Init();
	static void DeInit();
	static list<TCHAR *> GetFilterList();
	static int UpdateFilterList(list<TCHAR *> filterList);
	static int SaveFilter();
	static int AddFilter(TCHAR* szFilter);
	static int IsItemInFilterList(TCHAR *contact);
	static int FreeFilterList();
private:
	static list<TCHAR *> m_filterList;
	static 	int FilterHelper::InitFilterList();
	static void GetFilterPath(TCHAR *szPath);
};
