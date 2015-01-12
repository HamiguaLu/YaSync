#pragma once
#include "PEProtocol.h"
#include "MAPIEx.h"



#include "FilterHelper.h"


extern GUID MOBITNT_ITEM_ID_GUID;
extern GUID MOBITNT_THREAD_ID_GUID;
extern GUID MOBITNT_SMS_PHONE_NO_GUID;
extern GUID MOBITNT_DEVICE_MODEL_GUID;
extern GUID MOBITNT_CALL_NUMBER_GUID;
extern GUID MOBITNT_RECV_TYPE_GUID;
extern GUID MOBITNT_CALENDAR_ID_GUID;
extern GUID MOBITNT_MMS_TYPE_GUID;

extern GUID MOBITNT_NEED_SYNC_TAG_GUID;
extern GUID MOBITNT_FOLDER_BIN_GUID;
extern GUID MOBITNT_CONFLICT_TAG_GUID;

extern GUID MOBITNT_CONTACTS_TYPE_GUID;



#define PR_HTML										PROP_TAG( PT_BINARY, 0x1013)
#define PR_INTERNET_CPID							0x3FDE0003


class COutLookHelper
{
public:
	COutLookHelper(void);
	~COutLookHelper(void);

	virtual BOOL openFolder() = 0;

	static BOOL LogInEx(TCHAR *szPst);
	static BOOL LogIn();
	static void LogOut(void);
	
	BOOL openFolderByID(TCHAR *szFolderID);
	virtual void ApplyFilter(){};
	virtual int Export(){return PE_RET_FAIL;};
	virtual void RemoveDupliacte(){};

	int GetLastSyncTime(FILETIME &ftLastSyncTime);

	static list<TCHAR*> GetSubFolderList(int iFolderType);
	static list<CONTACT_ITEM*> GetContactList();

	static list<TCHAR*> GetStoreList();
	static TCHAR *GetDefaultProfile();

	static void TCHAR2SBinary(TCHAR *pszEntry,SBinary &bin);

	LPMESSAGE FindItemByEntryID(SBinary &bin);
	void DeleteItemByEntryID(SBinary &bin);
	
protected:

	static CMAPIEx *m_pMAPIMgr;
	LPMAPIFOLDER m_pFolder;

	virtual int GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft){return PE_RET_FAIL;};

	void SBinary2TCHAR(SBinary &bin,TCHAR *pszEntry);
	void GenFolderName(TCHAR *szFolderName,TCHAR *szFolderType,TCHAR *szPhoneName);

	CString m_sFolderName;

	ULONG m_lTag4ItemID;
	ULONG m_lTag4RecvType;
	ULONG m_lTag4PhoneNo;

	int m_iDefaultFolder;
	
	virtual list<TCHAR*> GetSubFolderList(){list<TCHAR*> subFolderList;return subFolderList;};
};


