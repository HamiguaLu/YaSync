#ifndef __PROTOCOL_HEADER_FILE_
#define __PROTOCOL_HEADER_FILE_
#include <WinSock2.h>

#include <list>
using namespace std;


#define _DETAIL_DEBUG_		0

#define IN
#define OUT

enum _PE_RET_DEF
{
	PE_RET_OK =	0,
	PE_RET_FAIL,
	PE_RET_NOT_ENOUGH_MEM,
	PE_RET_INVALID_PARA,
	PE_RET_NETWORK_ERROR,
	PE_RET_DATA_END,
	PE_RET_ALREADY_EXIST,
	PE_RET_ITEM_SAVED,
	PE_RET_WRONG_PWD,
	PE_RET_CMP_GREATER,
	PE_RET_CMP_LOWER,
	PE_RET_CMP_EQUAL,
	PE_RET_DEVICE_NOT_CONNECTED,
	PE_RET_NEED_EXIT,
	PE_RET_NEED_UPDATE_PC,
};



enum _YASYNC_CMD
{
	PE_CMD_NEW_SMS = 1,
	PE_CMD_HELP,
	PE_CMD_BUY,
	PE_CMD_THREAD,
	PE_CMD_DELETE,
	PE_CMD_SAVE_CONTACT,
	PE_CMD_FILTER,
	PE_CMD_RESTORE,
	PE_CMD_REPLY,
	PE_CMD_SHOW_UI,
	PE_CMD_STAT,
	PE_CMD_FILE_TRANSFER,
	PE_CMD_LOG,
	PE_CMD_SYNC,
};



#define		SUB_BTN_COLOR										RGB(238,47,65)
#define		SUB_BTN_SEL_COLOR									RGB(167,218,60)
#define		SUB_BTN_FONT_COLOR									RGB(255,255,255)
#define		SUB_BTN_FONT_FOCUS_COLOR							RGB(0,0,0)


#define		SUB_BTN1_COLOR										RGB(40,182,249)
#define		SUB_BTN1_SEL_COLOR									RGB(167,218,60)
#define		SUB_BTN1_FONT_COLOR									RGB(255,255,255)
#define		SUB_BTN1_FONT_FOCUS_COLOR							RGB(0,0,0)

#define		SUB_BTN2_COLOR										RGB(20,120,80)
#define		SUB_BTN2_SEL_COLOR									RGB(167,218,60)
#define		SUB_BTN2_FONT_COLOR									RGB(255,255,255)
#define		SUB_BTN2_FONT_FOCUS_COLOR							RGB(0,0,0)

#define		SUB_BTN3_COLOR										RGB(167,218,60)
#define		SUB_BTN3_SEL_COLOR									RGB(20,120,80)
#define		SUB_BTN3_FONT_COLOR									RGB(0,0,0)
#define		SUB_BTN3_FONT_FOCUS_COLOR							RGB(255,255,255)

#define		BTN_FONT_COLOR										RGB(122,121,121)
#define		BTN_BK_FOCUS_COLOR									RGB(255,255,255)
#define		BTN_BK_COLOR										RGB(248,248,248)

#define		MAIN_BTN_BK_COLOR									RGB(67,68,70)
#define		MAIN_BTN_FONT_COLOR									RGB(255,255,255)

#define		DLG_BK_COLOR										RGB(67,68,70)
#define		SUB_DLG_BK_COLOR									RGB(255,255,255)

#define		TRAY_DLG_BK_COLOR									RGB(255,255,223)
#define		TRAY_DLG_FG_COLOR									RGB(55,121,226)
#define		TRAY_BAR_BK_COLOR									RGB(255,153,0)

#define		MIN_BTN_COLOR										RGB(128,192,0)



enum _DEVICE_EVT_TYPE
{
	SYS_EVT_NONE = 100,
	SYS_EVT_SMS_CHANGED,
	SYS_EVT_CALL_LOG_CHANGED,
	SYS_EVT_CONTACT_CHANGED,
	SYS_EVT_CALENDAR_CHANGED,
	SYS_EVT_SMS_SENT_STATUS,
	SYS_EVT_SMS_DELIVER_STATUS,
	SYS_EVT_BATTERY_LEVEL_CHANGED
};

typedef struct _DEVICE_EVT
{
	int iEvtCode;
	int iHasMore;
	TCHAR szPara[255];
} DEVICE_EVT;



#define		PE_REMOVE_DUP_CALL									0
#define		PE_REMOVE_DUP_SMS									1

#define		MOBITNT_CONTACT_FOLDER_COMMENT						_T("Android Contact")
#define		MOBITNT_SMS_FOLDER_COMMENT							_T("Android SMS")
#define		MOBITNT_CALL_FOLDER_COMMENT							_T("Android Call log")

//#define WM_USER                         1024

enum _PE_MSG_DEF
{
	WM_PE_MIN_MSG = WM_USER + 1,
	WM_START_SRV,
	WM_STOP_SRV,
	WM_STOP_SRV2,
	WM_NEW_VER_FOUND,
	WM_DEVICE_CONNECTED,
	WM_DEVICE_DISCONNECTED,
	WM_EXPORT_ITEM,
	WM_DEVICE_NEW_SMS,
	WM_NOTIFYICON,
	WM_ADDIN_CMD,
	WM_SAVE_CONTACT,
	WM_GET_MSG_STORE,
	WM_MSG_STORE_COMING,
	WM_SMS_COMING,
	WM_CALL_COMING,
	WM_CONTACT_CHANGED,
	WM_SCAN_DEVICE,
	WM_ADD_FILTER,
	WM_APPLY_FILTER_NOW,
	WM_WRONG_SECURITY_CODE,
	WM_WRONG_CLIENT_VERSION,
	WM_WRONG_PC_VERSION,
	WM_RESTORE_ITEM,
	WM_INSTALL_CLIENT,
	WM_SYNC_CANCELLED,
	WM_GET_DEV_INFO,
	WM_CONNECT_2_DEVICE,
	WM_DISABLE_CONN_BTN,
	WM_DISABLE_SEND_BTN,
	WM_DEVICE_LIST_COMING,
	WM_UPDATE_DEVICE_LIST_ON_UI,
	WM_SWITCH_2_DEVICE,
	WM_SHOW_INSTALL_UI,
	WM_INSTALL_RESULT,
	WM_ASK_FOR_REG,
	WM_SYNC_NOW,
	WM_SYNC_STARTED,
	WM_SYNC_DONE,
	WM_GET_CAL_LIST,
	WM_CAL_LIST_COMING,
	WM_GET_ACCOUNTS,
	WM_ACCOUNTS_COMING,
	WM_GET_SUB_FOLDER,
	WM_SUB_FOLDER_COMING,
	WM_SYNC_INFO,
	WM_RESTORE_INFO,
	WM_GET_CONTACTS_FROM_OUTLOOK,
	WM_CONTACTS_COMING,
	WM_SEND_SMS,
	WM_SHOW_THREAD,
	WM_DELETE_SMS,
	WM_DELETE_ITEM,
	WM_REPLY_SMS,
	WM_SEND_SCHEDULE_SMS,
	WM_GET_SCHEDULE_SMS_LIST,
	WM_SCHEDULE_SMS_LIST_COMING,
	WM_DEL_SCHEDULE_SMS,
	WM_SEND_SMS_STATUS,
	WM_DELIEVER_SMS_STATUS,
	WM_QUERY_DEV_EVT,
	WM_BATTERY_LEVEL_CHANGED,
	WM_SHOW_ERROR_ON_UI,
	WM_ENABLE_SAVE,
	WM_SHOW_TRAY_INFO,
	WM_PE_MAX_MSG
};


#define YASYNC_MAIN_WIN_NAME		_T("YaSync-MobiTNT")


enum _SMS_STATUS
{
	PE_SEND_STATUS_CONN_DOWN,
	PE_SEND_STATUS_PHONE_INFO,
};

#define		PE_FILTER_FILE_NAME									_T("PEFilter.dat")
#define		PE_DEV_BLOCK_FILE_NAME								_T("PEPhoneBlock.dat")


#define		ONE_SECOND											(10 * 1000 * 1000)

#define		PE_LOG_FILE_NAME									_T("YaSync_Log.txt")

#define 	MAX_THREAD_SHOW_COUNT								(1024 * 10)
#define 	PE_MAX_BUF_LEN										(1024*1024)



enum _OUTLOOK_VER_DEF
{
	PE_OL_VER_UNKONW =	0,
	PE_OL_VER_2013,
	PE_OL_VER_2010,
	PE_OL_VER_2007,
	PE_OL_VER_2003,
};


enum _CONNECTION_TYPE
{
	PE_CONNECTION_TYPE_UNKONW = 0,
	PE_CONNECTION_TYPE_ADB,
	PE_CONNECTION_TYPE_WIFI,
	PE_CONNECTION_TYPE_BLUETOOTH
};


enum _CONNECTION_STATE
{
	PE_CONNNETION_STATE_UNKONW	= 0,
	PE_CONNNETION_STATE_OFFLINE,
	PE_CONNNETION_STATE_ONLINE
};


enum _SYNC_TYPE
{
	PE_SYNC_ALL	= 0,
	PE_SYNC_SMS,
	PE_SYNC_MMS,
	PE_SYNC_CALL,
	PE_SYNC_CONTACT,
	PE_SYNC_CALENDAR,
	PE_SYNC_CONTACT_2_PHONE,
	PE_SYNC_CAL_2_PHONE,
};


typedef struct _SYNC_STAT
{
	int iSmsCount;
	int iMmsCount;
	int iCallCount;
	int iContactsCount;
	int iCalendarCount;
} SYNC_STAT;



#define		PE_ANDROID_SVR_PORT									48300


enum _RESTORE_TYPE
{
	PE_ACT_SAVE_SMS_2_PHONE = 0,
	PE_ACT_SAVE_CALL_LOG_2_PHONE,
	PE_ACT_SAVE_CONTACT_2_PHONE
};


#define		CSV_MAX_ITEM_LEN									1024

#define		TRAY_ICON_OFFLINE									0
#define		TRAY_ICON_ONLINE									1
#define		TRAY_ICON_SYNC										2


enum _EMAIL_TYPE
{
	EMAIL_TYPE_HOME	= 1,
	EMAIL_TYPE_WORK,
	EMAIL_TYPE_OTHER,
	EMAIL_TYPE_MOBILE
};

enum _ADDR_TYPE
{
	ADDR_TYPE_UNKNOW = 0,
	ADDR_TYPE_HOME = 1,
	ADDR_TYPE_WORK = 2,
	ADDR_TYPE_OTHER = 3
};

enum _IM_TYPE
{
	IM_PROTOCOL_CUSTOM	= -1,
	IM_PROTOCOL_AIM	= 0,
	IM_PROTOCOL_MSN	= 1,
	IM_PROTOCOL_YAHOO = 2,
	IM_PROTOCOL_SKYPE = 3,
	IM_PROTOCOL_QQ = 4,
	IM_PROTOCOL_GOOGLE_TALK	= 5,
	IM_PROTOCOL_ICQ	= 6,
	IM_PROTOCOL_JABBER = 7,
	IM_PROTOCOL_NETMEETING = 8
};

enum _CALL_TYPE
{
	CALL_TYPE_INCOMING_CALL	= 1,
	CALL_TYPE_OUTGOING_CALL,
	CALL_TYPE_MISSED_CALL,
	CALL_TYPE_VOICE_MAIL,
	CALL_TYPE_REJECTED,
	CALL_TYPE_REFUSED,
};

//this is for android
enum _PHONE_TYPE
{
	PHONE_TYPE_UNKONW = 0,
	PHONE_TYPE_HOME,
	PHONE_TYPE_MOBILE,
	PHONE_TYPE_WORK,
	PHONE_TYPE_FAX_WORK,
	PHONE_TYPE_FAX_HOME,
	PHONE_TYPE_PAGER,
	PHONE_TYPE_OTHER,
	PHONE_TYPE_CALLBACK,
	PHONE_TYPE_CAR,
	PHONE_TYPE_COMPANY_MAIN,
	PHONE_TYPE_ISDN,
	PHONE_TYPE_MAIN,
	PHONE_TYPE_OTHER_FAX,
	PHONE_TYPE_RADIO,
	PHONE_TYPE_TELEX,
	PHONE_TYPE_TTY_TDD,
	PHONE_TYPE_WORK_MOBILE,
	PHONE_TYPE_WORK_PAGER,
	PHONE_TYPE_ASSISTANT,
	PHONE_TYPE_MMS,
	PHONE_TYPE_MAX_VALUE,
};

enum _WEBSITE_TYPE
{
	WEBSITE_TYPE_HOMEPAGE = 1,
	WEBSITE_TYPE_BLOG,
	WEBSITE_TYPE_PROFILE,
	WEBSITE_TYPE_HOME,
	WEBSITE_TYPE_WORK,
	WEBSITE_TYPE_FTP,
	WEBSITE_TYPE_OTHER,
};


enum _SMS_READ_FLAG
{
	SMS_READ_FLAG_READ	= 0,
	SMS_READ_FLAG_UNREAD
};

enum _ANDROID_SMS_TYPE
{
	SMS_TYPE_RECEIVE = 1,
	SMS_TYPE_SENT,
	SMS_TYE_DRAFT,
};


#define 	MAX_SMS_CONTENT_LEN									(1024*100 * 5)


#define 	UNICODE_BOM											0xFEFF

#define 	PE_MAX_HTTP_DATA_LEN								(50 * 1024 * 1024)

enum _PE_ITEM_CMP_RESULT
{
	PE_DIFFERENT_ITEM,
	PE_SAME_ITEM,
	PE_CONFLICT_ITEM,
	PE_NEED_MERGE_ITEM,
	PE_NO_NEED_MERGE_ITEM
};


#define		APP_VER												206
#define		APP_CLIENT_VER										20
#define		EA_RESPONSE_LIST_SIZE								5

#pragma pack(1)

typedef struct _SBIN
{
	ULONG  cb;
	BYTE   pb[255];
} SBin;


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383


#define HKEY_PE_ROOT			_T("Software\\YaSync")
#define HKEY_PE_COMMON_SETTING	_T("Software\\YaSync\\Settings")
#define HKEY_PE_PHONE_SETTING	_T("Software\\YaSync\\DEVICE%s")
#define HKEY_PE_AUTO_START		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")



typedef struct _PE_COMMON_SETTING 
{
	DWORD dwVer;
	DWORD dwIsRegistered;
	DWORD dwTrialStartTime;
	DWORD dwAutoSync;
	DWORD dwAutoStart;
	DWORD dwExitAfterSync;
	DWORD dwUseDefaultPst;
	TCHAR szSN[255];
	TCHAR szPst[255];
} PE_COMMON_SETTING;


typedef struct  _PE_PHONE_SETTING
{	
	TCHAR szPhoneID[255];
	TCHAR szPhoneName[255];
	TCHAR szSecurityCode[255];
	TCHAR szDevIP[255];
	
	DWORD dwSyncContact;
	DWORD dwContactSyncType;
	DWORD dwEnableContactAccount;
	TCHAR szContactAccounts[1024];
	TCHAR szContactFolderNames[1024];

	DWORD dwSyncCalendar;
	DWORD dwCalSyncType;
	DWORD dwOnlySyncCalAfterTime;
	TCHAR szCalendarIDs[255];
	TCHAR szDefaultCalID[255];
	TCHAR szCalendarFolderNames[1024];
	FILETIME ftCalendarSyncAfter;
	
	DWORD dwSyncSMS;
	DWORD dwSyncMMS;
	DWORD dwSetReadFlag;
	DWORD dwShowSMSReprot;
	TCHAR szSMSFolderName[1024];
	
	DWORD dwSyncCall;
	TCHAR szCallFolderName[1024];

	//for running state only
	DWORD dwConnState;
	DWORD dwSDKVer;
}PE_PHONE_SETTING;


typedef struct _MIME_ITEM
{
	ULONG	ulPartID;
	TCHAR	szMimeType[255];
	TCHAR	szMimeName[255];
	TCHAR	*szText;
	byte	*MimeData;
	int		iMimeDataLen;
}MIME_ITEM;


typedef struct _MMS_ITEM
{
	ULONG 				ulThreadID;
	ULONG 				ulMsgID;
	ULONG 				ulReadFlag;
	TCHAR 				tszName[255];
	TCHAR 				tszPhone[255];
	TCHAR 				*pszBody;
	ULONG   			ulMsgType;
	FILETIME 			ftRecvTime;
	list<MIME_ITEM*>	*pMimeData;
}MMS_ITEM;

typedef struct _SMS_ITEM
{
	ULONG 		ulThreadID;
	ULONG 		ulMsgID;
	ULONG 		ulReadFlag;
	TCHAR 		tszName[255];
	TCHAR 		tszPhone[255];
	TCHAR 		*pszBody;
	ULONG   	ulMsgType;
	FILETIME 	ftRecvTime;
}SMS_ITEM;


typedef struct _SCHEDULE_SMS_ITEM
{
	ULONG ulID;
	TCHAR szTo[1024];
	TCHAR szContent[1024];
	FILETIME ftSendTime;
}SCHEDULE_SMS_ITEM;




typedef struct _CALL_LOG_ITEM
{
	ULONG	 	ulCallType;
	ULONG    	ulCallID;
	FILETIME 	ftStartTime;
	FILETIME 	ftEndTime;
	TCHAR  		szNumberW[255];
	TCHAR  		szNameW[255];
	DWORD		dwNumberType;
} CALL_LOG_ITEM;


typedef struct _CALL_LOG_STAT
{
	TCHAR 		szName[255];
	TCHAR 		szPhone[255];
	ULONG		ulInTime;
	ULONG   	ulOutTime;
	ULONG 		ulInCount;
	ULONG 		ulOutCount;
	ULONG 		ulMissedCount;
} CALL_LOG_STAT;


typedef struct _SMS_STAT
{
	TCHAR 		szName[255];
	TCHAR 		szPhone[255];
	ULONG 		ulInCount;
	ULONG 		ulOutCount;
} SMS_STAT;


typedef struct _PhoneItem
{
	int iPhoneType;
	TCHAR szPhoneNo[255];
} PhoneItem;


typedef struct _EMailItem
{
	int iType;
	TCHAR szEmail[255];
} EMailItem;


typedef struct _WebSiteItem
{
	int iType;
	TCHAR szURL[1024];
} WebSiteItem;



typedef struct _ImItem
{
	int iIMType;
	TCHAR szIM[255];
} ImItem;



typedef struct _AddrItem
{
	int	   iAddrType;

	TCHAR  szAddrStreet[255];
	TCHAR  szAddrCity[255];
	TCHAR  szAddrRegion[255];
	TCHAR  szAddrPostCode[255];
	//TCHAR  szPoBox[255];
	TCHAR  szCountry[255];
	TCHAR  szAddrFormatAddress[255];
} AddrItem;
#define ADDR_PROP_COUNT		5


typedef struct _CONTACT_ITEM
{
	ULONG ulContactID;

	ULONG ulPhotoID;
	byte	*PhotoData;
	int		iPhotoDataLen;

	TCHAR szFamilyName[255];
	TCHAR szMiddleName[255];
	TCHAR szGivenName[255];

	TCHAR szOrgCompany[255];
	TCHAR szOrgTitle[255];

	TCHAR *szNotes;

	TCHAR szDisplayName[255];

	SBin bin;

	//following 3 is used for sync purpose only
	SBin FolderBin;
	int	 iNeedSync;
	int	 iIsConflictItem;

	int	iVisible;
	int iIsSim;

	list<PhoneItem*>	*pPhones;
	list<EMailItem*>	*pEMails;
	list<AddrItem*>		*pAddrs;
	list<ImItem*>		*pIMs;
	list<WebSiteItem*>	*pSites; 

	FILETIME 			ftBirthDay;
} CONTACT_ITEM;


typedef struct _ACCOUNT_ITEM
{
	TCHAR szName[255];
	TCHAR szType[255];
} ACCOUNT_ITEM;



typedef struct _CAL_EVT_ITEM
{
	ULONG    	ulEvtID;
	ULONG	 	ulCalendarID;
	FILETIME 	ftDTSTART;
	FILETIME 	ftDTEND;
	time_t		tDuration;
	DWORD		dwAllDay;
	DWORD		dwHasAlarm;
	DWORD		dwIsRecur;
	DWORD		dwReminderDelta;
	DWORD		dwRecurring;
	DWORD		dwRecurrenceType;
	TCHAR  		szTitle[255];
	TCHAR  		szDesc[1024];
	TCHAR  		szTimeZone[255];
	TCHAR  		szLocation[255];
	TCHAR  		szRRULE[1024];

	SBin		bin;
} CAL_EVT_ITEM;


typedef struct CAL_ITEM
{
	ULONG	ulCalendarID;
	TCHAR	szCalName[255];
	TCHAR	szCalType[255];
	TCHAR	szDisplayName[255];
	TCHAR	szName[255];
	int		visible;
} CAL_ITEM;


extern void PEWriteLog2File(TCHAR *szLogInfo);

extern CRITICAL_SECTION cs4Log;
#define PEWriteLog(tszInfo)	do{ \
	EnterCriticalSection(&cs4Log);\
	TCHAR *tszLog = (TCHAR*)malloc(PE_MAX_HTTP_DATA_LEN);\
	_stprintf(tszLog,_T("[%S:%u]  %s"),__FILE__,__LINE__,tszInfo);\
	PEWriteLog2File(tszLog);\
	free(tszLog);\
	LeaveCriticalSection(&cs4Log);\
}while(0);


typedef struct _DataBuffer
{
	int 		iBufferLen;
	TCHAR 		*pBuffer;

	int 		iRawBufferLen;
	char 		*pRawBufffer;
} DataBuffer;

typedef struct _FILTER
{
	TCHAR 		szName[100];
	TCHAR 		szPhone[100];
} FILTER;


typedef struct _DELAY_DATA
{
	UINT delay;
	UINT Msg;
	WPARAM wParam;
	LPARAM lParam;
} DelayData;



typedef struct _MSG_Data_Truck
{
	WPARAM p1;
	WPARAM p2;
	WPARAM p3;
} MSG_Data_Truck;


typedef struct _IPC_Data_Transfer
{
	int iCmd;
	char *pData;
	int iDataLen;
} IPC_Data_Transfer;




/*Following defines is used to communicate between PECore */
#define 	PE_UI_TYPE_APP										101
#define 	PE_UI_TYPE_PHOTO									102
#define 	PE_UI_TYPE_FILE										103


enum _E_SYNC_TYPE
{
	PE_SYNC_TYPE_SMS =	0,
	PE_SYNC_TYPE_CALL_LOG,
	PE_SYNC_TYPE_CONTACT,
	PE_SYNC_TYPE_SIM_CONTACT,
	PE_SYNC_TYPE_CONTACT_PC_TO_PHONE,
	PE_SYNC_TYPE_MMS,
	PE_SYNC_TYPE_CALENDAR,
	PE_SYNC_TYPE_CALENDAR_PC_TO_PHONE
};

enum _E_SYNC_DIRECTION
{
	PE_SYNC_BOTH_WAY =	0,
	PE_SYNC_FROM_PHONE_2_PC,
	PE_SYNC_FROM_PC_2_PHONE,
};



enum _E_FOLDER_TYPE
{
	FOLDER_TYPE_UNKONW = 0,
	FOLDER_TYPE_CONTACT,
	FOLDER_TYPE_CALENDAR,
	FOLDER_TYPE_SMS,
	FOLDER_TYPE_CALL_LOG
};



typedef struct _PE_DEV_INFO
{
	TCHAR szDevIP[255];
	TCHAR szPhoneID[255];
	TCHAR szPhoneModel[255];
	TCHAR szPhoneManufacturer[255];
	TCHAR szAppVer[255];
	TCHAR szOSVer[255];
	//DWORD dwCpuFreq;
	//TCHAR szCpuModel[255];
	int	  iAuthState;
	__int64 dwSDCardAvailableSpace;
	__int64 dwSDCardTotalSpace;
	__int64 dwAvailRAM;
	__int64 dwTotalRAM;
	//DWORD dwBatteryLevel;
	DWORD dwSDKVer;
}PE_DEV_INFO;



typedef struct _PE_SYNC_STATUS
{
	int iSyncType;
	float fTotalCount;
	float fCurCount;
}PE_SYNC_STATUS;


typedef struct _PE_RESTORE_STATUS
{
	int iRestoreType;
	float fTotalCount;
	float fCurCount;
} PE_RESTORE_STATUS;


#define CONTACT_ROOT_FOLDER_NAME	_T("Contact Root")
#define CALENDAR_ROOT_FOLDER_NAME	_T("Calendar Root")


#define HTTP_CONNECTION_TIME_OUT			(1000 * 10)
#define HTTP_SEND_TIME_OUT					(1000 * 60 * 3)
#define HTTP_RECV_TIME_OUT					(1000 * 60 * 3)


#define TRAY_TIMEER_EVT 0

typedef struct _TRAY_MSG
{
	int iType;
	TCHAR *szMsg;
	TCHAR *szReplyAddr;
}TRAY_MSG;

#define TRAY_MSG_NORMAL					0
#define TRAY_MSG_REPLY_SMS				1



#pragma pack()

void GenFolderName(TCHAR *szFolderName,TCHAR *szFolderType,TCHAR *szPhoneName);

#endif

