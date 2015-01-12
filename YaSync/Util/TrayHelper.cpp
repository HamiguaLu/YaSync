
#include "stdafx.h"
#include "TrayHelper.h"
#include "PEInterface.h"
#include "AppTools.h"
#include "SettingHelper.h"
#include "resource.h"

HICON g_hOnIcon;
HICON g_hOffIcon;
//HICON g_hSyncIcon;

NOTIFYICONDATA g_Tray;
CRITICAL_SECTION cs4TrayDlg;


DWORD UpdateTrayIcon(int iType)
{
	int iFirst = 0;
	if (g_Tray.hWnd == 0)
	{
		iFirst = 1;
		g_Tray.cbSize	= sizeof (NOTIFYICONDATA);

		g_Tray.hWnd	= GetMainUIHandle();

		g_Tray.uID	= ID_PE_TRAY;
		g_Tray.uFlags	= NIF_MESSAGE|NIF_ICON|NIF_TIP|NIF_INFO;
		g_Tray.uCallbackMessage	= WM_NOTIFYICON;
		g_Tray.dwState			= 0;				
		g_Tray.dwStateMask		= 0;				
		g_Tray.uVersion			= NOTIFYICON_VERSION;
		g_Tray.dwInfoFlags		= NIIF_USER;
		_tcscpy_s(g_Tray.szInfoTitle,_T("YaSync"));
		_tcscpy_s(g_Tray.szInfo,_T(""));

		g_hOffIcon = ::LoadIcon(GetAppInstance(),MAKEINTRESOURCE(IDI_OFFLINE));
		g_hOnIcon = ::LoadIcon(GetAppInstance(),MAKEINTRESOURCE(IDR_MAINFRAME));
	}

	if (iType == TRAY_ICON_OFFLINE && g_Tray.hIcon == g_hOffIcon)
	{
		return 0;
	}
	
	if (iType == TRAY_ICON_ONLINE && g_Tray.hIcon == g_hOnIcon)
	{
		return 0;
	}

	_tcscpy_s(g_Tray.szInfo,_T(""));

	if (iType == TRAY_ICON_OFFLINE)
	{
		g_Tray.hIcon = g_hOffIcon;
	}
	else if (iType == TRAY_ICON_ONLINE)
	{
		g_Tray.hIcon = g_hOnIcon;
	}

	g_Tray.dwInfoFlags=NIIF_INFO;
	
	if (iFirst)
	{
		Shell_NotifyIcon(NIM_ADD, &g_Tray);
	}
	else
	{
		Shell_NotifyIcon(NIM_MODIFY, &g_Tray);
	}
	return 0;
}


DWORD ShowTrayInfo(TCHAR* szInfo)
{
	if (_tcslen(szInfo) > 255)
	{
		szInfo[255] = 0;
	}

	_tcscpy_s(g_Tray.szInfo,szInfo);
	g_Tray.uFlags	= NIF_MESSAGE|NIF_ICON|NIF_TIP|NIF_INFO;

	Shell_NotifyIcon(NIM_MODIFY,&g_Tray);
	return 0;
}

void RemoveTray()
{
	Shell_NotifyIcon(NIM_DELETE,&g_Tray);
}


