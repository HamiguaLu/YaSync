package mobitnt.media;

import java.util.ArrayList;
import java.util.List;

import mobitnt.util.EAUtil;
import android.database.Cursor;
import android.provider.MediaStore;

public class EAVideoProvider{
	static int m_iCount = 0;

	public static int getCount() {
		if (m_iCount > 0){
			return m_iCount;
		}
		
		Cursor cursor = EAUtil.GetContentResolver().query(
				MediaStore.Video.Media.EXTERNAL_CONTENT_URI, null, null, null,
				null);

		if (cursor == null) {
			return 0;
		}

		m_iCount = cursor.getCount();
		cursor.close();

		return m_iCount;
	}

	public static List<EAVideo> getList(int iFrom, int iTo) {
		List<EAVideo> list = null;
		if (EAUtil.GetEAContext() == null || iTo < iFrom) {
			return null;
		}

		Cursor cursor = EAUtil.GetContentResolver().query(
				MediaStore.Video.Media.EXTERNAL_CONTENT_URI, null, null, null,
				null);
		if (cursor == null) {
			return null;
		}

		if (iFrom > cursor.getCount()) {
			return null;
		}

		if (iTo > cursor.getCount()) {
			iTo = cursor.getCount();
		}

		cursor.moveToPosition(iFrom);

		list = new ArrayList<EAVideo>();
		while (iFrom < iTo) {
			int id = cursor.getInt(cursor
					.getColumnIndexOrThrow(MediaStore.Video.Media._ID));
			String title = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Video.Media.TITLE));
			String album = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Video.Media.ALBUM));
			String artist = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Video.Media.ARTIST));
			String displayName = cursor
					.getString(cursor
							.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME));
			String path = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Video.Media.DATA));
			
		/*	String mimeType = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Video.Media.MIME_TYPE));
			long duration = cursor.getInt(cursor
					.getColumnIndexOrThrow(MediaStore.Video.Media.DURATION));
			long size = cursor.getLong(cursor
					.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE));*/
			EAVideo video = new EAVideo(id, title, album, artist, displayName,path);
			list.add(video);
			
			if (!cursor.moveToNext()){
				break;
			}
			
			++iFrom;
		}
		cursor.close();

		return list;
	}
}
