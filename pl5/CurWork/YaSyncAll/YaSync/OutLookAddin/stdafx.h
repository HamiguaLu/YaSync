// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__CBA35630_11B7_4467_928C_D14EDE24BA49__INCLUDED_)
#define AFX_STDAFX_H__CBA35630_11B7_4467_928C_D14EDE24BA49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

#include <comutil.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
// Adding Outlook architecture and outlook library
// This changes according to ur system configuration
// This is pertaining to Microsoft Office XP
// For Office 2000 pls refer my notes
#import "C:\Program Files\Common Files\Microsoft Shared\Office12\mso.dll" rename_namespace("Office"), named_guids \
	rename("RGB", "RGB") \
	rename("DocumentProperties","DocumentProperties")
using namespace Office;

#import "C:\Program Files\Microsoft Office\Office12\MSOUTL.olb" rename_namespace("Outlook"), named_guids, raw_interfaces_only \
	rename("CopyFile", "CopyFile") \
	rename("PlaySound","PlaySound")
#include <windows.h>

using namespace Outlook;
#endif // !defined(AFX_STDAFX_H__CBA35630_11B7_4467_928C_D14EDE24BA49__INCLUDED)
