#pragma once
#include "PEProtocol.h"
#include "MAPIEx.h"


#include "OutLookHelper.h"

#include "DeviceAgent.h"

#define PidLidWorkAddressStreet 				0x8045
#define PidLidWorkAddressCity 					0x8046 
#define PidLidWorkAddressState					0x8047 
#define PidLidWorkAddressPostalCode 			0x8048 
#define PidLidWorkAddressCountry 				0x8049 
#define	PidLidFileUnder							0x8005

#define PidLidEmail1EmailAddress				0x8083
#define PidLidEmail2EmailAddress				0x8093
#define PidLidEmail3EmailAddress				0x80A3

#define PidLidInstantMessagingAddress			0x8062

#define PidLidWebSite							0x802B

#define PidLidHasPicture						0x8015


#define PR_ATTACHMENT_CONTACTPHOTO PROP_TAG( PT_BOOLEAN, 0x7FFF)

class DeviceAgent;


class ContactsHelper:public COutLookHelper
{
public:
	BOOL openFolder();
	BOOL CreateFolder4SimContact();
	int ContactCompare(CONTACT_ITEM  *pContact,CONTACT_ITEM  *pContact2);
	int IsContactExistInPhoneFolder(CONTACT_ITEM *pContact);
	int SaveContact2OutLook(CONTACT_ITEM  *pContact);
	int Export();
	int DeleteContactByEntryID(ULONG cb,LPBYTE lpb);
	int FindContactItemByEntryID(TCHAR *tszEntryID,CONTACT_ITEM  *pContact);
	static void FreeContactItem(CONTACT_ITEM *pContact);

	static int IsPhoneTypeSupported(int iPhoneType);

	int SyncContact2Phone(TCHAR *szSubFolder);

	//int EmulateContact();

	void RemoveDupliacte();
	list<TCHAR*> GetSubFolderList();
	list<CONTACT_ITEM*> GetContactList();
protected:
	int GenContactProperty(LPMESSAGE lpMessage);
	bool GetContactProperty(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	bool GetContactType(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	bool SetContactProperty(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);

	int SetContactFileUnder(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactName(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactAddress(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactPhone(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactOrg(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactEMail(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactIM(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);

	int SaveImage(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	
	int PhoneListCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM  *pContact2);
	int AddrListCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM  *pContact2);
	int EMailListCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM  *pContact2);
	int IMListCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM  *pContact2);

	int GetContactName(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int GetContactOrg(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int GetContactEMail(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int GetContactPhone(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int GetContactAddress(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int GetContactWebSite(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactNotes(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactBirthDay(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);
	int SetContactWebSite(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact);

	int SyncContactFromFolder2Phone();


	ULONG m_lTag4ContactType;

};


