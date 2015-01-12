package mobitnt.net;

import java.util.Properties;
import mobitnt.util.EADefine;
import mobitnt.util.MobiTNTLog;


public class PageGen {
	public int m_iReqType;
	public static String m_sXmlHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n";
	
	String m_sRespMimeType = NanoHTTPD.MIME_XML;

	PageGen() {

	}

	public String GetRespMimeType(){
		return m_sRespMimeType;
	}
	
	public void SetRespMimeType(String sRespMimeType){
		m_sRespMimeType = sRespMimeType;
	}
	
	public static String ReturnException(String sVal) {
		String sXml = String.format("<EARetException>%s</EARetException>", sVal);
		return sXml;
	}
	
	static public String GenRetXml(String sVal,int iCode){
		String sFmt = "<EARet><RetCode>%d</RetCode><RetString>%s</RetString></EARet>";
		String sXml = m_sXmlHeader;
		MobiTNTLog.write(sVal);
		sXml += String.format(sFmt, iCode,sVal);
		return sXml;
	}

	public static String GenRefreshCmd(){
		return "<Refresh>refresh</Refresh>";
	}
	
	public static String GenRetCode(int iCode) {
		String sXml = String.format("<EARetCode>%s</EARetCode>",iCode);
		return sXml;
	}
	
	public String ProcessRequest(String sReq, Properties parms) {
		return GenRetCode(EADefine.EA_RET_UNKONW_REQ);
	}
}
