package mobitnt.media;

import java.util.ArrayList;
import java.util.List;

import mobitnt.util.EAUtil;
import android.database.Cursor;
import android.provider.MediaStore;

public class EAImageProvider{
	
	static int m_iCount = 0;
	
	public static int getCount() {
		if (m_iCount > 0){
			return m_iCount;
		}
		
		String[] projection = {MediaStore.Images.Media.DATA};  
		Cursor cursor = EAUtil.GetContentResolver().query( MediaStore.Images.Media.EXTERNAL_CONTENT_URI, projection, null, null, MediaStore.Images.Media._ID);
		if (cursor == null)	{
			return 0;
		}
		
		m_iCount = cursor.getCount();
		cursor.close();
		
		return m_iCount;
	}
	
	public static List<EAImage> getList(int iFrom, int iTo) {
		List<EAImage> list = null;
		if (EAUtil.GetEAContext() == null || iTo < iFrom) {
			return null;
		}
		
		Cursor cursor = EAUtil.GetContentResolver().query(
				MediaStore.Images.Media.EXTERNAL_CONTENT_URI, null, null, null,
				"_id Desc");
		if (cursor == null) {
			return null;
		}
		
		if (iFrom > cursor.getCount()){
			return null;
		}
		
		if (iTo > cursor.getCount()){
			iTo = cursor.getCount();
		}
		
		if (!cursor.moveToPosition(iFrom)){
			return null;
		}
		
		list = new ArrayList<EAImage>();
		while ( iFrom <= iTo ) {
			String galleryName = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Images.Media.BUCKET_DISPLAY_NAME));
			if ( (galleryName == null) || !(galleryName.equalsIgnoreCase("Camera") ) ){
				if (!cursor.moveToNext()){
					break;
				}
				continue;
			}
			
			int id = cursor.getInt(cursor
					.getColumnIndexOrThrow(MediaStore.Images.Media._ID));
			String title = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Images.Media.TITLE));
			
			String path = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Images.Media.DATA));
			String displayName = cursor
					.getString(cursor
							.getColumnIndexOrThrow(MediaStore.Images.Media.DISPLAY_NAME));
	/*		String mimeType = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Images.Media.MIME_TYPE));
			long size = cursor.getLong(cursor
					.getColumnIndexOrThrow(MediaStore.Images.Media.SIZE));*/
			
			
			EAImage img = new EAImage(id, title, displayName, galleryName,path);
			list.add(img);
			
			if (!cursor.moveToNext()){
				break;
			}
			
			++iFrom;
		}
		
		cursor.close();

		return list;
	}
}

