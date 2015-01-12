package mobitnt.net;

import java.util.Properties;
import mobitnt.android.data.ContactDetailInfo;
import mobitnt.android.wrapper.ContactApi;
import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;

public class ContactManager extends PageGen {
	public String ProcessRequest(String request, Properties parms) {
		try {
			String sAction = parms.getProperty(EADefine.EA_ACTION_TAG, "n");

			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_CONTACT_LIST)) {
				String sFrom = parms.getProperty(EADefine.EA_FROM_TAG, "0");
				String sAccountName = parms.getProperty(EADefine.EA_ACCOUNT_NAME_TAG, "");
				
				return ContactApi.GetContactList(sFrom,sAccountName);
			}
			
			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_CONTACT_GROUP_LIST)) {
				return ContactApi.GetContactGroupList();
			}
			
			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_CONTACT_COUNT)) {
				int iTotalCount = ContactApi.GetContactCount();
				String sItemFmt = "<ItemCount>%d</ItemCount>";
				String sXml = String.format(sItemFmt, iTotalCount);
				return sXml;
			}
			
			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_MAX_CONTACT_ID)) {
				String sXml = m_sXmlHeader;
				sXml += "<MaxID>";
				if (ContactContentObserver.m_iChanged == 0 || ContactContentObserver.m_iChanged == 2){
					String sMaxID = ContactApi.GetMaxContactID();
					sXml += sMaxID;
					ContactContentObserver.m_iChanged = 1;
				}
				else{
					// don't sync if there is no change detected
					sXml += "0";
				}
					
				sXml += "</MaxID>";
				return sXml;
			}
			
			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_CHECK_NEW_CONTACT)) {
				long lNewContact = 0;
		
				String sItemFmt = "<NewContact>%d</NewContact>\r\n";
				String sXml = m_sXmlHeader;
				sXml += String.format(sItemFmt, lNewContact);
				return sXml;
			}
			
			if(sAction.equalsIgnoreCase(EADefine.EA_ACT_DELETE_CONTACT)) {
				
				String sID = parms.getProperty(EADefine.EA_ID_TAG, "0");
				ContactApi.delete(Integer.parseInt(sID));
				return GenRetCode(EADefine.EA_RET_OK);
			}
			
			if (sAction.contains(EADefine.EA_ACT_INSERT_CONTACT) || sAction.contains(EADefine.EA_ACT_UPDATE_CONTACT)){
				String sID = parms.getProperty(EADefine.EA_ID_TAG, "0");
				String sFamilyName = parms.getProperty(EADefine.EA_FAMILY_NAME_TAG, "");
				String sMiddleName = parms.getProperty(EADefine.EA_MIDDLE_NAME_TAG, "");
				String sGivenName = parms.getProperty(EADefine.EA_GIVEN_NAME_TAG, "");
				String sDisplayName = parms.getProperty(EADefine.EA_DISPLAY_NAME_TAG, "");
				String sPhone = parms.getProperty(EADefine.EA_CONTACT_PHONE_TAG, "");
				String sAddr = parms.getProperty(EADefine.EA_CONTACT_ADDR_TAG, "");//use ; to divide each item
				String sOrg = parms.getProperty(EADefine.EA_CONTACT_ORG_TAG, "");
				String sCMail = parms.getProperty(EADefine.EA_CONTACT_MAIL_TAG, "");
				String sNotes = parms.getProperty(EADefine.EA_CONTACT_NOTES_TAG, "");
				String sIM = parms.getProperty(EADefine.EA_CONTACT_IM_TAG, "");
				String sWebSite = parms.getProperty(EADefine.EA_CONTACT_WEBSITE_TAG, "");
				
				sID = EAUtil.CHECK_STRING(sID, "0");
				
				ContactDetailInfo c = ContactApi.ContactParser(sID,sFamilyName,sMiddleName,sGivenName,sDisplayName, sPhone,sAddr, sOrg, sCMail, sNotes, sIM,sWebSite);
				int iRet = EADefine.EA_RET_FAILED;
				if (sAction.contains(EADefine.EA_ACT_INSERT_CONTACT)){
					iRet = ContactApi.insertContact(c);
				}else if (sAction.contains(EADefine.EA_ACT_UPDATE_CONTACT)){
					iRet = ContactApi.update(c);
				}else{
					return GenRetCode(EADefine.EA_RET_UNKONW_REQ);
				}
	
				return GenRetCode(iRet);
			}
			
		} catch (Exception e) {
			return GenRetXml(
					"Error occured while getting contact list:" + e.toString(),
					0);
		}
		
		return GenRetCode(EADefine.EA_RET_UNKONW_REQ);
	}
	

}
