#pragma once
#include "PEProtocol.h"
#include "MAPIEx.h"

#include "OutLookHelper.h"




#define PidLidPhoneCallStartTime			0x8706
#define PidLidPhoneCallEndTime				0x8708
#define PidLidPhoneCallTypeDescription		0x8712
#define PidLidPhoneCallType					0x8700
#define PidLidPhoneCallDuration				0x8707
#define PidLidContacts						0x853A

#define PR_ICON_INDEX						0x10800003


class CallLogHelper:public COutLookHelper
{
public:
	BOOL openFolder();
	int Export();
	void ApplyFilter();
	int SaveCallLog2OutLook(CALL_LOG_ITEM& call);
	int FindCallItemByEntryID(TCHAR *tszEntryID,CALL_LOG_ITEM& call);

	int IsItemExist(CALL_LOG_ITEM *pCall);

	void RemoveDupliacte();

	list<TCHAR*> GetSubFolderList();

	int GenStat(FILETIME &ftStart,FILETIME &ftEnd,CALL_LOG_STAT *pStat);
	
protected:
	int GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft);
	
	ULONG m_lTag4CallNumber;
	void FormatCallLog(CALL_LOG_ITEM& call,CString &szBody,CString &szSubject);
	HRESULT SetCallCategories(LPMESSAGE lpMessage,CALL_LOG_ITEM& callData);
	HRESULT SetCallPropertyEx(LPMESSAGE lpMessage,CALL_LOG_ITEM& callData);
	int GetCallPropertyEx(LPMESSAGE lpMessage,CALL_LOG_ITEM& callData);
	int GenCallProperty(LPMESSAGE lpMessage);
	bool GetCallProperty(LPMESSAGE lpMessage,CALL_LOG_ITEM& call);
	bool SetCallProperty(LPMESSAGE lpMessage,CALL_LOG_ITEM& call);
	HRESULT SetCallContact(LPMESSAGE lpMessage,CALL_LOG_ITEM& callData);


};



