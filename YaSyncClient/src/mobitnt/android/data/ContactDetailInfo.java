package mobitnt.android.data;

import java.util.List;



public class ContactDetailInfo {
	public String sDisplayName;
	public String sFamilyName;
	public String sMiddleName;
	public String sGivenName;
	
	public long lID;
	public List<String> phoneList;
	public List<String> EMailList;
	public List<String> AddrList;
	public List<String> OrgList;
	public List<String> ImList;
	public List<String> NotesList;
	public List<String> WebSiteList;
	
	public ContactDetailInfo(){
		sFamilyName = "";
		sMiddleName = "";
		sGivenName = "";
		lID = 0;
		phoneList = null;
		EMailList = null;
		AddrList = null;
		OrgList = null;
		ImList = null;
		NotesList = null;
		WebSiteList = null;
	}
}
