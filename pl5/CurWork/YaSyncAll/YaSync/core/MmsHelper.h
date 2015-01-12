#include "PEProtocol.h"
#include "MAPIEx.h"


#include "SmsHelper.h"




class MmsHelper:public SmsHelper
{
public:
	int SaveMMS2OutLook(MMS_ITEM* pMmsMsg);
	static void FreeMimeItem(MMS_ITEM *pMms);
	int GetMaxMmsID(ULONG &ulMaxMsgID);
protected:
	int GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft);
	void GetSmsFromMms(MMS_ITEM *pMms,SMS_ITEM *pSms);
	int SaveMimeImage(LPMESSAGE lpMessage,MMS_ITEM *pMms);
	int SaveMimeText(LPMESSAGE lpMessage,MMS_ITEM *pMms);

	int GenMmsTypeProperty(LPMESSAGE lpMessage);
	int SetMmsTypeProperty(LPMESSAGE lpMessage);
	int IsMmsType(LPMESSAGE lpMessage);

	ULONG m_lTag4MmsType;
};





