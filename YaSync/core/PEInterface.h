#ifndef __EXPORT_BASE_INTERFACE__
#define __EXPORT_BASE_INTERFACE__

#include "windows.h"

void SetMainUIHandle(HWND hWnd);

HWND GetMainUIHandle();

DWORD PE_Init();

DWORD PEStartSvr();

DWORD PEStopSvr();

DWORD PEIsRegistered();

#endif
