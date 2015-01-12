#pragma once
#include "PEProtocol.h"
#include "MAPIEx.h"


#include "OutLookHelper.h"


class SmsHelper:public COutLookHelper
{
public:
	BOOL openFolder();
	int SaveSMS2OutLook(SMS_ITEM& smsMsg);
	
	int GetMaxMsgID(ULONG &ulMaxMsgID);
	int Export();
	void ApplyFilter();
	list<SMS_ITEM*> GetSmsListByThreadID(TCHAR *tszFolderID,ULONG ulThreadID,TCHAR *szPhoneID);
	int FindSmsIemByEntryID(TCHAR *tszEntryID,SMS_ITEM& sms);
	void RemoveDupliacte();

	
	list<TCHAR*> GetSubFolderList();

	int GenStat(FILETIME &ftStart,FILETIME &ftEnd,SMS_STAT *pStat);

	
protected:
	ULONG m_lTag4ThreadID;
	
	int GenSmsProperty(LPMESSAGE lpMessage);
	HRESULT SetSmsPropertyEx(LPMESSAGE lpMessage,SMS_ITEM& sms);
	bool GetSmsPropertyEx(LPMESSAGE lpMessage,SMS_ITEM& sms);
	bool SetSmsProperty(LPMESSAGE lpMessage,SMS_ITEM& sms);
	bool GetSmsProperty(LPMESSAGE lpMessage,SMS_ITEM& sms);

	int GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft);
};


