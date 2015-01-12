package mobitnt.net;

import java.util.List;
import java.util.Properties;

import mobitnt.util.*;


import mobitnt.media.EAAudio;
import mobitnt.media.EAAudioProvider;
import mobitnt.media.EAImage;
import mobitnt.media.EAImageProvider;
import mobitnt.media.EAVideo;
import mobitnt.media.EAVideoProvider;

/**
 * @author hamigua
 * 
 */
public class MediaManager extends PageGen {
	String GetImageList(int iFrom, int iTo) {
		int iCount = EAImageProvider.getCount();
		if (iFrom >= iCount || iFrom > iTo || iCount == 0) {
			return GenRetCode(EADefine.EA_RET_END_OF_FILE);
		}

		if (iTo > iCount) {
			iTo = iCount;
		}

		List<EAImage> PhotoList = (List<EAImage>) EAImageProvider.getList(
				iFrom, iTo);
		if (PhotoList == null) {
			return "<MediaList></MediaList>";
		}

		int iMediaCount = PhotoList.size();
		StringBuilder sList = new StringBuilder("<MediaList>");
		
		for (int i = 0; i < iMediaCount; ++i) {
			EAImage v = PhotoList.get(i);
			
			sList.append("<Media>");
			
			sList.append("<Path>");
			sList.append(v.getPath());
			sList.append("</Path>");
			
			sList.append("<Title>");
			sList.append(v.getTitle());
			sList.append("</Title>");
			
			sList.append("<DisplayName>");
			sList.append(v.getDisplayName());
			sList.append("</DisplayName>");
			
			sList.append("<GalleryName>");
			sList.append(v.getGalleryName());
			sList.append("</GalleryName>");
			
			sList.append("</Media>");
			
		}

		sList.append("</MediaList>");

		return sList.toString();

	}
	
	String GetAudioList(int iFrom, int iTo) {
		int iCount = EAAudioProvider.getCount();
		if (iFrom >= iCount || iFrom > iTo || iCount == 0) {
			return GenRetCode(EADefine.EA_RET_END_OF_FILE);
		}

		if (iTo > iCount) {
			iTo = iCount;
		}

		List<EAAudio> AudioList = (List<EAAudio>) EAAudioProvider.getList(iFrom, iTo);
		if (AudioList == null) {
			return "<MediaList></MediaList>";
		}

		int iMediaCount = AudioList.size();
		StringBuilder sList = new StringBuilder("<MediaList>");
		
		for (int i = 0; i < iMediaCount; ++i) {
			EAAudio v = AudioList.get(i);
			
			sList.append("<Media>");
			
			sList.append("<Path>");
			sList.append(v.getPath());
			sList.append("</Path>");
			
			sList.append("<Title>");
			sList.append(v.getTitle());
			sList.append("</Title>");
			
			sList.append("<DisplayName>");
			sList.append(v.getDisplayName());
			sList.append("</DisplayName>");
			
			sList.append("<Artist>");
			sList.append(v.getArtist());
			sList.append("</Artist>");
			
			sList.append("</Media>");
			
		}

		sList.append("</MediaList>");

		return sList.toString();

	}

	
	String GetVideoList(int iFrom, int iTo) {
		int iCount = EAVideoProvider.getCount();
		if (iFrom >= iCount || iFrom > iTo || iCount == 0) {
			return GenRetCode(EADefine.EA_RET_END_OF_FILE);
		}

		if (iTo > iCount) {
			iTo = iCount;
		}

		List<EAVideo> VideoList = (List<EAVideo>) EAVideoProvider.getList(
				iFrom, iTo);
		if (VideoList == null) {
			return "<MediaList></MediaList>";
		}

		int iMediaCount = VideoList.size();
		StringBuilder sList = new StringBuilder("<MediaList>");
		
		for (int i = 0; i < iMediaCount; ++i) {
			EAVideo v = VideoList.get(i);
			
			sList.append("<Media>");
			
			sList.append("<Path>");
			sList.append(v.getPath());
			sList.append("</Path>");
			
			sList.append("<Title>");
			sList.append(v.getTitle());
			sList.append("</Title>");
			
			sList.append("<DisplayName>");
			sList.append(v.getDisplayName());
			sList.append("</DisplayName>");
			
			sList.append("<Artist>");
			sList.append(v.getArtist());
			sList.append("</Artist>");
			
			sList.append("</Media>");
			
		}

		sList.append("</MediaList>");

		return sList.toString();

	}


	/*
	 * URL format :action=getfile&&file=sdcard/aa
	 */
	public String ProcessRequest(String request, Properties parms) {

		int iFrom = 0;
		int iTo = 0;

		String sAction = parms.getProperty(EADefine.EA_ACTION_TAG, EADefine.EA_ACT_GET_IMAGE_LIST);
		sAction = sAction.toLowerCase();
		
		if (sAction.equals(EADefine.EA_ACT_GET_IMAGE_COUNT)) {
			int iTotalCount = EAImageProvider.getCount();
			String sItemFmt = "<ItemCount>%d</ItemCount>";
			return String.format(sItemFmt, iTotalCount);
		}
		
		if (sAction.equals(EADefine.EA_ACT_GET_VIDEO_COUNT)) {
			int iTotalCount = EAVideoProvider.getCount();
			String sItemFmt = "<ItemCount>%d</ItemCount>";
			return String.format(sItemFmt, iTotalCount);
		}
		
		if (sAction.equals(EADefine.EA_ACT_GET_AUDIO_COUNT)) {
			int iTotalCount = EAAudioProvider.getCount();
			String sItemFmt = "<ItemCount>%d</ItemCount>";
			return String.format(sItemFmt, iTotalCount);
		}
		
		String sFrom = parms.getProperty(EADefine.EA_FROM_TAG, "0");
		String sTo = parms.getProperty(EADefine.EA_TO_TAG, "0");

		iFrom = Integer.valueOf(sFrom);
		iTo = Integer.valueOf(sTo);
		
		if (sAction.equals(EADefine.EA_ACT_GET_IMAGE_LIST)) {
			return GetImageList(iFrom, iTo);
		} 
		
		if (sAction.equals(EADefine.EA_ACT_GET_VIDEO_LIST)) {
			return GetVideoList(iFrom, iTo);
		}
		
		if (sAction.equals(EADefine.EA_ACT_GET_AUDIO_LIST)) {
			return GetAudioList(iFrom, iTo);
		}
		
		return GenRetCode(EADefine.EA_RET_FAILED);
	}

}
