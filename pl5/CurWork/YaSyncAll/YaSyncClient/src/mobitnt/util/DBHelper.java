package mobitnt.util;

import android.content.ContentValues;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

/**
 * @author admin
 * 
 */

public class DBHelper extends SQLiteOpenHelper {
	private static final String DB_NAME = "YaSyncScheduleSms.db";
	private static final int DB_VERSION = 2;
	private static final String TB_NAME = "ScheduleSms";
	
	private SQLiteDatabase mydb;

	public DBHelper() {
		super(EAUtil.eaContext, DB_NAME, null, DB_VERSION);
	}


	@Override
	public void onCreate(SQLiteDatabase db) {
		mydb = db;
		try {
			StringBuffer sql = new StringBuffer();
			sql.append("CREATE TABLE ").append(TB_NAME).append(" (");
			sql.append("id INTEGER DEFAULT '1' NOT NULL PRIMARY KEY AUTOINCREMENT,");
			sql.append("ToList TEXT NOT NULL,");
			sql.append("Content TEXT NOT NULL,");
			sql.append("ScheduleTime TEXT NOT NULL)");
			mydb.execSQL(sql.toString());
		} catch (Exception e) {
			return;
		}
	}

	@Override
	public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
		try {
			mydb = db;
			StringBuffer sql = new StringBuffer();
			sql.append("DROP TABLE IF EXISTS ").append(TB_NAME);
			mydb.execSQL(sql.toString());
			onCreate(mydb);
		} catch (Exception e) {

		}
	}

	/**
	 * 成功返回刚刚新增记录的rowId，否则返回-1
	 * 
	 * @param userInfo
	 * @return
	 */
	public long insert(String sToList, String sContent, String sScheduleTime) {
		try {
			// db的打开已经实现了缓存
			SQLiteDatabase db = getWritableDatabase();
			ContentValues values = new ContentValues();
			values.put("ToList", sToList);
			values.put("Content", sContent);
			values.put("ScheduleTime", sScheduleTime);
			long lRet = db.insert(TB_NAME, null, values); 
			return lRet;
		} catch (Exception e) {
			return 0;
		}
	}

	public void clearTable() {
		try {
			SQLiteDatabase db = getWritableDatabase();
			db.delete(TB_NAME, null, null);
		} catch (Exception e) {

		}
	}

	public void delete(String recordId) {
		try {

			SQLiteDatabase db = getWritableDatabase();
			// db.delete(TB_NAME, "id='"+userId+"'", null);
			db.delete(TB_NAME, "id=?", new String[] { recordId });
		} catch (Exception e) {
	
		}
	}

	public Cursor query() {
		try {
			SQLiteDatabase db = getWritableDatabase();
			String[] columns = { "id", "ToList", "Content", "ScheduleTime" };
			Cursor c = db.query(TB_NAME, columns, null, null, null, null,
					"ScheduleTime desc");
			return c;
		} catch (Exception e) {
			return null;
		}
	}
	
	public void close() {  
		if (mydb != null)  
			mydb.close();
			mydb = null;
		}  

	
}
