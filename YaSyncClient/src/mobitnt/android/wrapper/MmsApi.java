package mobitnt.android.wrapper;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.text.MessageFormat;

import mobitnt.net.PageGen;
import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;
import android.content.ContentResolver;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;

public class MmsApi {

	public static String GetMmsList(String sStartDate) {
		ContentResolver contentResolver = EAUtil.GetContentResolver();
		final String[] projection = new String[] { "*" };
		Uri uri = Uri.parse("content://mms");
		String sortOrder = "date ASC";
		
		String selection = "date >= ?";
		String[] selectionArgs = new String[] { sStartDate };
				
		Cursor cur = contentResolver.query(uri, projection, selection, selectionArgs, sortOrder);
		if (cur == null) {
			return null;
		}

		if (!cur.moveToFirst()) {
			cur.close();
			return null;
		}
		
		StringBuilder sXml = new StringBuilder(PageGen.m_sXmlHeader);
		sXml.append("<MMSList>");
		int iCount = 0;
		do {
			String sType = cur.getString(cur.getColumnIndex("ct_t"));
			if (sType == null || !sType.equals("application/vnd.wap.multipart.related")) {
				// not mms
				continue;
			}

			// it's MMS
			if (++iCount > EADefine.EA_RESPONSE_LIST_SIZE){
				break;
			}
			
			sXml.append("<MMS>");
			String[] colNames = cur.getColumnNames();
			for (int i = 0; i < colNames.length; ++i) {
				String sVal = cur.getString(i);
				if (sVal == null) {
					continue;
				}

				sVal = sVal.trim();
				if (sVal.length() > 0) {
					sXml.append("<" + colNames[i] + ">");
					sXml.append(EAUtil.EncodeItem(cur.getString(i)));
					sXml.append("</" + colNames[i] + ">");
				}
			}
			
			String sID = cur.getString(cur.getColumnIndex("_id"));
				
			String selectionPart = "mid=" + sID;
			Uri parturi = Uri.parse("content://mms/part");
			Cursor cursor = contentResolver.query(parturi, null,selectionPart, null, null);
			if (cursor.moveToFirst()) {
			    do {
			    	sXml.append("<Parts>");
			    	
			    	String[] PartcolNames = cursor.getColumnNames();
					for (int i = 0; i < PartcolNames.length; ++i) {
						String sVal = cursor.getString(i);
						if (sVal == null) {
							continue;
						}

						sVal = sVal.trim();
						if (sVal.length() > 0) {
							sXml.append("<" + PartcolNames[i] + ">");
							sXml.append(EAUtil.EncodeItem(cursor.getString(i)));
							sXml.append("</" + PartcolNames[i] + ">");
						}
					}
					sXml.append("</Parts>");
			    } while (cursor.moveToNext());
			}
			
			String sPhoneNo = getAddressNumber(sID);
			sXml.append("<PhoneNo>");
			sXml.append(EAUtil.EncodeItem(sPhoneNo));
			sXml.append("</PhoneNo>");
			
			String sName = ContactApi.getContactNameByPhoneNum(sPhoneNo);
			sXml.append("<Name>");
			sXml.append(EAUtil.EncodeItem(sName));
			sXml.append("</Name>");
			
			sXml.append("</MMS>");


		} while (cur.moveToNext());
		sXml.append("</MMSList>");
		cur.close();
		return sXml.toString();
	}

	public static String getMmsText(String id) {
		Uri partURI = Uri.parse("content://mms/part/" + id);
		InputStream is = null;
		StringBuilder sb = new StringBuilder();
		try {
			is = EAUtil.GetContentResolver().openInputStream(partURI);
			if (is != null) {
				InputStreamReader isr = new InputStreamReader(is, "UTF-8");
				BufferedReader reader = new BufferedReader(isr);
				String temp = reader.readLine();
				while (temp != null) {
					sb.append(temp);
					temp = reader.readLine();
				}
			}
		} catch (IOException e) {
		} finally {
			if (is != null) {
				try {
					is.close();
				} catch (IOException e) {
				}
			}
		}
		return sb.toString();
	}

	public static Bitmap getMmsImage(String _id) {
		Uri partURI = Uri.parse("content://mms/part/" + _id);
		InputStream is = null;
		Bitmap bitmap = null;
		try {
			is = EAUtil.GetContentResolver().openInputStream(partURI);
			bitmap = BitmapFactory.decodeStream(is);
		} catch (IOException e) {
		} finally {
			if (is != null) {
				try {
					is.close();
				} catch (IOException e) {
				}
			}
		}
		return bitmap;
	}

	public static String getAddressNumber(String id) {
		String selectionAdd = new String("msg_id=" + id);
		String uriStr = MessageFormat.format("content://mms/{0}/addr", id);
		Uri uriAddress = Uri.parse(uriStr);
		Cursor cur = EAUtil.GetContentResolver().query(uriAddress, null,
				selectionAdd, null, null);

		if (cur == null) {
			return null;
		}

		if (!cur.moveToFirst()) {
			cur.close();
			return null;
		}

		String name = null;
		do {
			String number = cur.getString(cur.getColumnIndex("address"));
			if (number == null) {
				continue;
			}
			
			if (number.equals("insert-address-token")){
				continue;
			}
			
			String type = cur.getString(cur.getColumnIndex("type"));
			//If it is an outbound message, the "type" to look for will be 137. For an inbound message, the "type" will be 151
			if (type.equals("137") || type.equals("151")){
				name = number.replace("-", "");
				break;
			}
		} while (cur.moveToNext());

		cur.close();

		return name;
	}
	
}
