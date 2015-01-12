package mobitnt.media;

import java.util.ArrayList;
import java.util.List;

import android.database.Cursor;
import android.provider.MediaStore;
import mobitnt.util.EAUtil;

public class EAAudioProvider  {
	static int m_iCount = 0;

	public static int getCount() {
		if (m_iCount > 0){
			return m_iCount;
		}
		
		Cursor cursor = EAUtil.GetContentResolver().query(
				MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, null, null, null,
				null);

		if (cursor == null) {
			return 0;
		}

		m_iCount = cursor.getCount();
		cursor.close();

		return m_iCount;
	}

	public static List<EAAudio> getList(int iFrom, int iTo) {
		List<EAAudio> list = null;
		if (EAUtil.GetEAContext() == null || iTo < iFrom) {
			return null;
		}

		Cursor cursor = EAUtil.GetContentResolver().query(
				MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, null, null, null,
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

		list = new ArrayList<EAAudio>();
		while (iFrom < iTo) {
			int id = cursor.getInt(cursor
					.getColumnIndexOrThrow(MediaStore.Audio.Media._ID));
			String title = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Audio.Media.TITLE));
			String album = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Audio.Media.ALBUM));
			String artist = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Audio.Media.ARTIST));
			String path = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Audio.Media.DATA));
			String displayName = cursor
					.getString(cursor
							.getColumnIndexOrThrow(MediaStore.Audio.Media.DISPLAY_NAME));
	/*		String mimeType = cursor.getString(cursor
					.getColumnIndexOrThrow(MediaStore.Audio.Media.MIME_TYPE));
			long duration = cursor.getInt(cursor
					.getColumnIndexOrThrow(MediaStore.Audio.Media.DURATION));
			long size = cursor.getLong(cursor
					.getColumnIndexOrThrow(MediaStore.Audio.Media.SIZE));*/
			EAAudio audio = new EAAudio(id, title, album, artist, path,
					displayName);
			list.add(audio);
			
			if (!cursor.moveToNext()){
				break;
			}

			++iFrom;
		}
		
		cursor.close();

		return list;
	}
}

