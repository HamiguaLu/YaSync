package mobitnt.net;

import java.net.URLDecoder;
import java.net.URLEncoder;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.Properties;

import mobitnt.android.data.FileProperty;
import mobitnt.android.wrapper.FileApi;
import mobitnt.util.*;

/**
 * @author hamigua
 * 
 */
public class StorageManager extends PageGen {
	// String m_sCurrentFolder;

	
	String GetVirtualPath(String sRealPath){
		String sExternalRoot = FileApi.getExternalStoragePath();
		sRealPath = sRealPath.replace(sExternalRoot, "/");
		sRealPath = sRealPath.replace("//", "/");
		return sRealPath;
	}
	
	String GetSDCardList(){
		HashSet<String> sdCards = FileApi.getExternalMounts();
		String sXml = "<SdCardList>";
		if (sdCards != null){
			Iterator<String> itr = sdCards.iterator();
			while(itr.hasNext()){
				sXml += "<path>";
				sXml += "<value>";
				sXml += URLEncoder.encode(itr.next().toString());
				sXml += "</value>";
				sXml += "<def>1</def>";
				sXml += "</path>";
			}	
		}
		
		sXml += "</SdCardList>";
		return sXml;
	}

	public String ProcessRequest(String request, Properties parms) {
		try {
			int iFrom = 0;
			String sFilePath = "/";

			String sAction = parms.getProperty(EADefine.EA_ACTION_TAG, EADefine.EA_ACT_GET_FOLDER_LIST);
			sFilePath = parms.getProperty(EADefine.EA_FILE_TAG, "n");

			if (sAction.contains(EADefine.EA_ACT_GET_SDCARD_LIST)) {
				return GetSDCardList();
			}

			if (sAction.contains(EADefine.EA_ACT_GET_FOLDER_LIST)) {
				String sFrom = parms.getProperty(EADefine.EA_FROM_TAG, "0");

				iFrom = Integer.valueOf(sFrom);
				return GetFolderList(sFilePath, iFrom);
			}

			if (sAction.contains(EADefine.EA_ACT_REMOVE_FILE)) {
				String sFileList = parms.getProperty(EADefine.EA_FILE_LIST_TAG, "n");
				sFileList = URLDecoder.decode(sFileList, "UTF-8");
				
				if (sFileList.equals("n")){
					return GenRetCode(EADefine.EA_RET_FAILED);
				}
				
				String[] sFiles = sFileList.split(",");
				for (int i = 0; i < sFiles.length; ++i){
					if (sFiles[i].length() <= 1){
						continue;
					}
					
					String sPath = sFiles[i];
										
					if (!FileApi.RemoveFile(sPath)) {
						return GenRetCode(EADefine.EA_RET_FAILED);
					}
				}
				
				return GenRefreshCmd();
			}
			
			if (sAction.contains(EADefine.EA_ACT_CREATE_FILE)) {
				if (FileApi.CreateFolder(sFilePath)) {
					return GenRefreshCmd();
				}
				return GenRetCode(EADefine.EA_RET_FAILED);
			}
			
			if (sAction.contains(EADefine.EA_ACT_RENAME_FILE)) {
				String sNewPath = parms.getProperty(EADefine.EA_NEW_PATH_TAG, "n");
				if (FileApi.Rename(sFilePath, sNewPath)) {
					return GenRefreshCmd();
				}
				return GenRetCode(EADefine.EA_RET_FAILED);
			}
			
			if (sAction.contains(EADefine.EA_ACT_GET_EXTERNAL_ROOT_FOLDER)) {
				String sExtPath = FileApi.getExternalStoragePath();
				if (sExtPath.length() <= 1){
					return GenRetCode(EADefine.EA_RET_NO_EXTERNAL_STOREAGE);
				}

				return "<RootPath>" + sExtPath + "</RootPath>";
			}
			
		} catch (Exception e) {
			e.printStackTrace();
			return ReturnException(e.toString());
		}
		
		return GenRetCode(EADefine.EA_RET_UNKONW_REQ);

	}

	String GetFolderList(String sFolderPath, int iFrom) {
		List<FileProperty> files = FileApi.GetFolderList(sFolderPath, iFrom);
		if (files == null || files.size() < 1) {
			if (iFrom > 0){
				return GenRetCode(EADefine.EA_RET_END_OF_FILE);	
			}
		}

		int iTotalCount = FileApi.GetFileCountInFolder(sFolderPath);

		StringBuilder sXml = new StringBuilder(PageGen.m_sXmlHeader);
		sXml.append("<FolderList>");
		sXml.append("<CurrentFolder>" + sFolderPath + "</CurrentFolder>");
		sXml.append("<TotalCount>" + Integer.toString(iTotalCount)
				+ "</TotalCount>");
		sXml.append("<Folders>");
		if (files != null){
			for (int i = 0; i < files.size(); ++i) {
				sXml.append("<Folder>");
				String sFileName = /* m_sCurrentFolder + */files.get(i).m_sFileName;
				sXml.append("<Name>" + sFileName + "</Name>");

				sXml.append("<Size>" + files.get(i).m_iFileSize + "</Size>");

				String sType = String.format(Locale.ENGLISH,"<Type>%d</Type>",
						files.get(i).m_iFileType);
				sXml.append(sType);

				sXml.append("</Folder>");
			}		
		}
	
		sXml.append("</Folders>");
		sXml.append("</FolderList>");

		return sXml.toString();
	}
}
