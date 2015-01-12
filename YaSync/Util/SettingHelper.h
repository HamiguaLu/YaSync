#pragma once

#include "PEProtocol.h"
#include "atlstr.h"
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/timeb.h>

class CSettingHelper
{
public:
	static void Init(void);
	CSettingHelper(void);
	~CSettingHelper(void);

	static void SetAutoStartApp(int iEnable);
	static int GetLastPhoneName(TCHAR *szPhoneName);
	static void SetLastPhoneName(TCHAR *szPhoneName);

	static void SetCommonSetting(PE_COMMON_SETTING *setting);
	static void ResetCommonSetting(PE_COMMON_SETTING *setting);
	static int GetCommonSetting(PE_COMMON_SETTING *setting);

	static void SetPhoneSetting(PE_PHONE_SETTING *setting);
	static int ResetPhoneSetting(PE_PHONE_SETTING *setting);
	static int ResetMediaSyncSetting(PE_PHONE_SETTING *setting);
	static int GetPhoneSetting(PE_PHONE_SETTING *setting);

	static void SetDeviceConnState();
	static int GetDeviceConnState();

	static int RemovePhoneInfo(TCHAR *szPhoneName);
	static list<PE_PHONE_SETTING*> GetPhoneInfoList();
};
