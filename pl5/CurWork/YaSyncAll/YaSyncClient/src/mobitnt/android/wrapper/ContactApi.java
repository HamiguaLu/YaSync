package mobitnt.android.wrapper;

import java.util.ArrayList;
import mobitnt.android.data.*;
import mobitnt.net.PageGen;
import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;
import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.RemoteException;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Nickname;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.CommonDataKinds.Im;
import android.provider.ContactsContract.CommonDataKinds.Note;
import android.provider.ContactsContract.CommonDataKinds.Organization;
import android.provider.ContactsContract.PhoneLookup;
import android.provider.ContactsContract.RawContacts;
import android.util.Log;

public class ContactApi {

	public static final int PHONE_TYPE_HOME = 1;
	public static final int PHONE_TYPE_MOBILE = 2;
	public static final int PHONE_TYPE_WORK = 3;

	public static final int EMAIL_TYPE_HOME = 1;
	public static final int EMAIL_TYPE_WORK = 2;
	public static final int EMAIL_TYPE_OTHER = 3;
	public static final int EMAIL_TYPE_MOBILE = 4;

	public static final String SIM_CONTACTS_URI = "content://icc/adn";

	static String GetNameByPhoneNum(String sPhone) {
		String contact = "";
		if (sPhone.length() < 1) {
			return "";
		}

		try {
			Uri uri = Uri.withAppendedPath(PhoneLookup.CONTENT_FILTER_URI,
					Uri.encode(sPhone));
			Cursor cs = EAUtil.GetContentResolver().query(uri,
					new String[] { PhoneLookup.DISPLAY_NAME },
					PhoneLookup.NUMBER + "='" + sPhone + "'", null, null);
			if (cs == null || !cs.moveToFirst()) {
				if (cs != null) {
					cs.close();
				}
				return contact;
			}

			contact = cs.getString(cs.getColumnIndex(PhoneLookup.DISPLAY_NAME));
			cs.close();

			return contact;
		} catch (Exception e) {
			Log.e("Contact", e.toString());
			return "";
		}

	}

	static public String getContactNameByPhoneNum(String sPhone) {
		if (sPhone == null || sPhone.length() < 1) {
			return "";
		}

		// String[] sPhoneList = sPhones.split(" ");
		// for (int i = 0; i < sPhoneList.length; ++i) {
		String sName = GetNameByPhoneNum(sPhone);
		if (sName != null && sName.length() >= 1) {
			return sName;
		}
		// }

		return sPhone;

	}

	static public String GetMaxSimContactID() {
		ContentResolver cr = EAUtil.GetContentResolver();
		Uri simUri = Uri.parse(SIM_CONTACTS_URI);
		Cursor cur = cr.query(simUri, null, null, null, "_id DESC");

		try {
			if (cur == null || !cur.moveToFirst()) {
				if (cur != null) {
					cur.close();
				}
				return "0";
			}

			int iCallIDIndex = cur.getColumnIndexOrThrow("_id");
			String sID = cur.getString(iCallIDIndex);
			cur.close();

			return sID;

		} catch (Exception e) {
			cur.close();
			return "0";
		}
	}

	static public String GetSimContacts(String sFromID) {
		ContentResolver resolver = EAUtil.GetContentResolver();
		final String sortOrder = "_id ASC";

		String selection = "_id >= ? ";
		String[] selectionArgs = new String[] { sFromID };

		Uri simUri = Uri.parse(SIM_CONTACTS_URI);
		Cursor cur = resolver.query(simUri, null, selection, selectionArgs,
				sortOrder);
		if (cur == null || !cur.moveToFirst()) {
			if (cur != null) {
				cur.close();
			}
			return null;
		}

		// List<ContactInfo> clist = new ArrayList<ContactInfo>();
		StringBuilder sXml = new StringBuilder();
		int iCount = 0;
		sXml.append("<Contacts>");
		do {
			if (iCount++ > EADefine.EA_RESPONSE_LIST_SIZE) {
				break;
			}

			sXml.append("<Contact>");
			sXml.append(EAUtil.Cursor2Xml(cur));
			sXml.append("</Contact>");

		} while (cur.moveToNext());

		cur.close();

		sXml.append("</Contacts>");

		return sXml.toString();
	}

	static public String GetMaxContactID() {
		ContentResolver cr = EAUtil.GetContentResolver();
		Cursor cur = cr.query(ContactsContract.Contacts.CONTENT_URI, null,
				null, null, "_id DESC");

		try {
			if (cur == null || !cur.moveToFirst()) {
				if (cur != null) {
					cur.close();
				}
				return "0";
			}

			int iCallIDIndex = cur.getColumnIndexOrThrow("_id");
			String sID = cur.getString(iCallIDIndex);
			cur.close();

			return sID;

		} catch (Exception e) {
			cur.close();
			return "0";
		}
	}

	static public int GetContactCount() {
		Cursor cur = EAUtil.GetContentResolver().query(
				ContactsContract.Contacts.CONTENT_URI, null, null, null, null);
		if (cur == null || !cur.moveToFirst()) {
			if (cur != null) {
				cur.close();
			}
			return 0;
		}

		int iCount = cur.getCount();
		cur.close();
		return iCount;
	}

	static public Bitmap GetPhoto(String PhotoID) {
		Uri photoUri = ContentUris.withAppendedId(
				ContactsContract.Data.CONTENT_URI, Long.parseLong(PhotoID));

		Cursor c = EAUtil.GetContentResolver().query(photoUri,
				new String[] { ContactsContract.CommonDataKinds.Photo.PHOTO },
				null, null, null);
		if ((c == null) || (!c.moveToFirst())) {
			if (c != null) {
				c.close();
			}
			return null;
		}

		byte[] data = c.getBlob(0);
		c.close();
		if (data == null || data.length < 1){
			return null;
		}

		Bitmap bitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
		return bitmap;
	}

	static String GetBirthday(String contactId) {
		Uri uri = ContactsContract.Data.CONTENT_URI;

		String[] projection = new String[] {
				ContactsContract.Contacts.DISPLAY_NAME,
				ContactsContract.CommonDataKinds.Event.CONTACT_ID,
				ContactsContract.CommonDataKinds.Event.START_DATE };

		String where = ContactsContract.Data.CONTACT_ID + " = ? " + " and "
				+ ContactsContract.Data.MIMETYPE + "= ? AND "
				+ ContactsContract.CommonDataKinds.Event.TYPE + "="
				+ ContactsContract.CommonDataKinds.Event.TYPE_BIRTHDAY;
		String[] selectionArgs = new String[] { contactId,
				ContactsContract.CommonDataKinds.Event.CONTENT_ITEM_TYPE };
		String sortOrder = null;
		Cursor cur = EAUtil.GetContentResolver().query(uri, projection, where,
				selectionArgs, sortOrder);
		if ((cur == null) || (!cur.moveToFirst())) {
			if (cur != null) {
				cur.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<Birthday>");
		do {
			try {
				int bDayColumn = cur
						.getColumnIndex(ContactsContract.CommonDataKinds.Event.START_DATE);
				String sDay = cur.getString(bDayColumn);

				sXml.append("<Day>");
				sXml.append(EAUtil.EncodeItem(sDay));
				sXml.append("</Day>");
			} catch (Exception e) {

			}
		} while (cur.moveToNext());

		sXml.append("</Birthday>");
		return sXml.toString();
	}

	static String GetWebsite(String contactId) {
		String[] cols = { ContactsContract.CommonDataKinds.Website.TYPE,
				ContactsContract.CommonDataKinds.Website.URL };
		String filter = ContactsContract.Data.CONTACT_ID + " = ? " + " and "
				+ ContactsContract.Data.MIMETYPE + " = '"
				+ ContactsContract.CommonDataKinds.Website.CONTENT_ITEM_TYPE
				+ "'";
		String[] params = { String.valueOf(contactId) };
		Cursor sites = EAUtil.GetContentResolver().query(
				ContactsContract.Data.CONTENT_URI, cols, filter, params, null);
		if ((sites == null) || (!sites.moveToFirst())) {
			if (sites != null) {
				sites.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<WebSites>");
		do {
			try {

				String type = sites
						.getString(sites
								.getColumnIndex(ContactsContract.CommonDataKinds.Website.TYPE));
				String url = sites
						.getString(sites
								.getColumnIndex(ContactsContract.CommonDataKinds.Website.URL));

				sXml.append("<WebSite>");
				sXml.append("<TYPE>");
				sXml.append(type);
				sXml.append("</TYPE>");

				sXml.append("<URL>");
				sXml.append(EAUtil.EncodeItem(url));
				sXml.append("</URL>");

				sXml.append("</WebSite>");
			} catch (Exception e) {

			}
		} while (sites.moveToNext());

		sXml.append("</WebSites>");

		sites.close();

		return sXml.toString();
	}

	static String GetEMail(String ContactID) {
		// 获取该联系人邮箱
		Cursor emails = EAUtil.GetContentResolver().query(
				ContactsContract.CommonDataKinds.Email.CONTENT_URI,
				null,
				ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = "
						+ ContactID, null, null);
		if ((emails == null) || (!emails.moveToFirst())) {
			if (emails != null) {
				emails.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<EMails>");
		do {
			// 遍历所有的电话号码
			String emailType = emails
					.getString(emails
							.getColumnIndex(ContactsContract.CommonDataKinds.Email.TYPE));

			String emailValue = emails
					.getString(emails
							.getColumnIndex(ContactsContract.CommonDataKinds.Email.ADDRESS));

			sXml.append("<EMail>");
			sXml.append("<TYPE>");
			sXml.append(emailType);
			sXml.append("</TYPE>");

			sXml.append("<ADDRESS>");
			sXml.append(EAUtil.EncodeItem(emailValue));
			sXml.append("</ADDRESS>");

			sXml.append("</EMail>");
		} while (emails.moveToNext());

		sXml.append("</EMails>");

		emails.close();

		return sXml.toString();

	}

	static String GetPhones(String ContactID) {

		// 获得联系人的电话号码
		Cursor phones = EAUtil.GetContentResolver().query(
				ContactsContract.CommonDataKinds.Phone.CONTENT_URI,
				null,
				ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = "
						+ ContactID, null, null);
		if (phones == null || !phones.moveToFirst()) {
			if (phones != null) {
				phones.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<Phones>");

		do {
			// 遍历所有的电话号码
			String phoneNumber = phones
					.getString(phones
							.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));
			String phoneType = phones
					.getString(phones
							.getColumnIndex(ContactsContract.CommonDataKinds.Phone.TYPE));
			sXml.append("<Phone>");

			sXml.append("<NUMBER>");
			sXml.append(EAUtil.EncodeItem(phoneNumber));
			sXml.append("</NUMBER>");

			sXml.append("<TYPE>");
			sXml.append(phoneType);
			sXml.append("</TYPE>");

			sXml.append("</Phone>");

		} while (phones.moveToNext());

		phones.close();

		sXml.append("</Phones>");

		return sXml.toString();
	}

	static String GetNickName(String ContactID) {

		// 获取nickname信息
		Cursor nicknames = EAUtil.GetContentResolver().query(
				Data.CONTENT_URI,
				new String[] { Data._ID, Nickname.NAME },
				Data.CONTACT_ID + "=?" + " AND " + Data.MIMETYPE + "='"
						+ Nickname.CONTENT_ITEM_TYPE + "'",
				new String[] { ContactID }, null);

		if (nicknames == null || !nicknames.moveToFirst()) {
			if (nicknames != null) {
				nicknames.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<NickNames>");

		do {
			String nickname_ = nicknames.getString(nicknames
					.getColumnIndex(Nickname.NAME));

			sXml.append("<NickName>");
			sXml.append(EAUtil.EncodeItem(nickname_));
			sXml.append("</NickName>");

		} while (nicknames.moveToNext());
		sXml.append("</NickNames>");

		return sXml.toString();

	}

	static String GetNotes(String ContactID) {
		// 获取备注信息
		Cursor notes = EAUtil.GetContentResolver().query(
				Data.CONTENT_URI,
				new String[] { Data._ID, Note.NOTE },
				Data.CONTACT_ID + "=?" + " AND " + Data.MIMETYPE + "='"
						+ Note.CONTENT_ITEM_TYPE + "'",
				new String[] { ContactID }, null);

		if (notes == null) {
			return "";
		}

		StringBuilder sXml = new StringBuilder();

		if (notes.moveToFirst()) {
			sXml.append("<Notes>");
			do {
				String noteinfo = notes.getString(notes
						.getColumnIndex(Note.NOTE));

				sXml.append("<NoteInfo>");
				sXml.append(EAUtil.EncodeItem(noteinfo));
				sXml.append("</NoteInfo>");

			} while (notes.moveToNext());
			sXml.append("</Notes>");
		}
		notes.close();

		return sXml.toString();
	}

	static String GetOrg(String ContactID) {
		// 获取该联系人组织
		Cursor orgCur = EAUtil.GetContentResolver().query(
				Data.CONTENT_URI,
				new String[] { Data._ID, Organization.COMPANY,
						Organization.TITLE },
				Data.CONTACT_ID + "=?" + " AND " + Data.MIMETYPE + "='"
						+ Organization.CONTENT_ITEM_TYPE + "'",
				new String[] { ContactID }, null);

		if (orgCur == null || !orgCur.moveToFirst()) {
			if (orgCur != null) {
				orgCur.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<ORGs>");

		do {
			String company = orgCur.getString(orgCur
					.getColumnIndex(Organization.COMPANY));
			String title = orgCur.getString(orgCur
					.getColumnIndex(Organization.TITLE));

			sXml.append("<ORG>");
			sXml.append("<company>");
			sXml.append(EAUtil.EncodeItem(company));
			sXml.append("</company>");

			sXml.append("<title>");
			sXml.append(EAUtil.EncodeItem(title));
			sXml.append("</title>");

			sXml.append("</ORG>");

		} while (orgCur.moveToNext());

		orgCur.close();
		sXml.append("</ORGs>");
		return sXml.toString();
	}

	static String GetAddr(String ContactID) {
		// 获取该联系人地址
		Cursor address = EAUtil.GetContentResolver().query(
				ContactsContract.CommonDataKinds.StructuredPostal.CONTENT_URI,
				null,
				ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = "
						+ ContactID, null, null);
		if ((address == null) || !address.moveToFirst()) {
			if (address != null) {
				address.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<Addrs>");
		do {
			// 遍历所有的地址
			sXml.append("<Addr>");

			String street = address
					.getString(address
							.getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.STREET));
			String city = address
					.getString(address
							.getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.CITY));
			String region = address
					.getString(address
							.getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.REGION));
			String postCode = address
					.getString(address
							.getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.POSTCODE));

			String poBox = address
					.getString(address
							.getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.POBOX));

			String country = address
					.getString(address
							.getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.COUNTRY));
			String type = address
					.getString(address
							.getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.TYPE));

			String formatAddress = address
					.getString(address
							.getColumnIndex(ContactsContract.CommonDataKinds.StructuredPostal.FORMATTED_ADDRESS));

			sXml.append("<street>");
			sXml.append(EAUtil.EncodeItem(street));
			sXml.append("</street>");

			sXml.append("<city>");
			sXml.append(EAUtil.EncodeItem(city));
			sXml.append("</city>");

			sXml.append("<region>");
			sXml.append(EAUtil.EncodeItem(region));
			sXml.append("</region>");

			sXml.append("<postCode>");
			sXml.append(EAUtil.EncodeItem(postCode));
			sXml.append("</postCode>");

			sXml.append("<poBox>");
			sXml.append(EAUtil.EncodeItem(poBox));
			sXml.append("</poBox>");

			sXml.append("<coutry>");
			sXml.append(EAUtil.EncodeItem(country));
			sXml.append("</coutry>");

			sXml.append("<type>");
			sXml.append(EAUtil.EncodeItem(type));
			sXml.append("</type>");
			sXml.append("<formatAddress>");
			sXml.append(EAUtil.EncodeItem(formatAddress));
			sXml.append("</formatAddress>");

			sXml.append("</Addr>");
		} while (address.moveToNext());

		address.close();
		sXml.append("</Addrs>");

		return sXml.toString();
	}

	static String GetIM(String ContactID) {
		// 获取该联系人IM
		Cursor IMs = EAUtil.GetContentResolver().query(
				Data.CONTENT_URI,
				new String[] { Data._ID, Im.PROTOCOL, Im.DATA },
				Data.CONTACT_ID + "=?" + " AND " + Data.MIMETYPE + "='"
						+ Im.CONTENT_ITEM_TYPE + "'",
				new String[] { ContactID }, null);

		if (IMs == null || !IMs.moveToFirst()) {
			if (IMs != null) {
				IMs.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<IMs>");

		do {
			String protocol = IMs.getString(IMs.getColumnIndex(Im.PROTOCOL));
			String data = IMs.getString(IMs.getColumnIndex(Im.DATA));

			sXml.append("<IM>");

			sXml.append("<PROTOCOL>");
			sXml.append(EAUtil.EncodeItem(protocol));
			sXml.append("</PROTOCOL>");

			sXml.append("<DATA>");
			sXml.append(EAUtil.EncodeItem(data));
			sXml.append("</DATA>");

			sXml.append("</IM>");
		} while (IMs.moveToNext());

		IMs.close();
		sXml.append("</IMs>");

		return sXml.toString();
	}

	static boolean IsContactExist(ContactDetailInfo c) {
		String whereName = ContactsContract.CommonDataKinds.StructuredName.DISPLAY_NAME
				+ " = ?";

		String[] whereNameParams = new String[] { c.sDisplayName };

		Cursor nameCur = EAUtil.GetContentResolver().query(
				ContactsContract.Data.CONTENT_URI, null, whereName,
				whereNameParams, null);
		if (nameCur == null || !nameCur.moveToFirst()) {
			if (nameCur != null) {
				nameCur.close();
			}
			return false;
		}

		/*
		 * int iCount = nameCur.getCount();
		 * 
		 * String given = nameCur .getString(nameCur
		 * .getColumnIndex(ContactsContract
		 * .CommonDataKinds.StructuredName.GIVEN_NAME)); String middle = nameCur
		 * .getString(nameCur
		 * .getColumnIndex(ContactsContract.CommonDataKinds.StructuredName
		 * .MIDDLE_NAME)); String family = nameCur .getString(nameCur
		 * .getColumnIndex
		 * (ContactsContract.CommonDataKinds.StructuredName.FAMILY_NAME));
		 */
		nameCur.close();
		return true;
	}

	static String GetName(String ContactID) {
		String whereName = ContactsContract.Data.MIMETYPE + " = ? AND "
				+ ContactsContract.CommonDataKinds.StructuredName.CONTACT_ID
				+ " = ?";
		String[] whereNameParams = new String[] {
				ContactsContract.CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE,
				ContactID };
		Cursor nameCur = EAUtil.GetContentResolver().query(
				ContactsContract.Data.CONTENT_URI, null, whereName,
				whereNameParams,
				ContactsContract.CommonDataKinds.StructuredName.GIVEN_NAME);
		if (nameCur == null || !nameCur.moveToFirst()) {
			if (nameCur != null) {
				nameCur.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<Names>");

		String given = nameCur
				.getString(nameCur
						.getColumnIndex(ContactsContract.CommonDataKinds.StructuredName.GIVEN_NAME));
		String middle = nameCur
				.getString(nameCur
						.getColumnIndex(ContactsContract.CommonDataKinds.StructuredName.MIDDLE_NAME));
		String family = nameCur
				.getString(nameCur
						.getColumnIndex(ContactsContract.CommonDataKinds.StructuredName.FAMILY_NAME));

		sXml.append("<GIVEN_NAME>");
		sXml.append(EAUtil.EncodeItem(given));
		sXml.append("</GIVEN_NAME>");

		sXml.append("<FAMILY_NAME>");
		sXml.append(EAUtil.EncodeItem(family));
		sXml.append("</FAMILY_NAME>");

		sXml.append("<MIDDLE_NAME>");
		sXml.append(EAUtil.EncodeItem(middle));
		sXml.append("</MIDDLE_NAME>");

		nameCur.close();

		sXml.append("</Names>");

		return sXml.toString();

	}

	static boolean IsContactBelongToAccount(String ContactID, String sAccount) {
		if (sAccount == null || sAccount.length() < 1) {
			// account means user want to sync all contacts
			return true;
		}
		try {
			Cursor cursor = null;
			cursor = EAUtil.GetContentResolver().query(
					ContactsContract.RawContacts.CONTENT_URI,
					new String[] { ContactsContract.RawContacts.ACCOUNT_NAME,
							ContactsContract.RawContacts.ACCOUNT_TYPE },
					ContactsContract.RawContacts.CONTACT_ID + "=? AND "
							+ ContactsContract.RawContacts.ACCOUNT_NAME + "=?",
					new String[] { ContactID, sAccount }, null);
			if ((cursor == null) || !cursor.moveToFirst()) {
				if (cursor != null) {
					cursor.close();
				}
				return false;
			}

			cursor.close();
			return true;

		} catch (Exception e) {
			// Utils.log(this.getClass().getName(), e.getMessage());
		}

		return false;
	}

	static public String GetContactList(String sFromID, String sAccount) {
		ContentResolver resolver = EAUtil.GetContentResolver();
		final String sortOrder = "_id ASC";

		String selection = "_id >= ? and ";
		selection += ContactsContract.Contacts.IN_VISIBLE_GROUP	+ " = ?";
		String[] selectionArgs = new String[] { sFromID, "1" };

		Cursor cur = resolver.query(ContactsContract.Contacts.CONTENT_URI,
				null, selection, selectionArgs, sortOrder);
		if (cur == null || !cur.moveToFirst()) {
			if (cur != null) {
				cur.close();
			}
			return null;
		}

		// List<ContactInfo> clist = new ArrayList<ContactInfo>();
		StringBuilder sXml = new StringBuilder(PageGen.m_sXmlHeader);
		int iCount = 0;
		sXml.append("<Contacts>");
		do {
			if (iCount++ > EADefine.EA_RESPONSE_LIST_SIZE) {
				break;
			}

			int idColumn = cur.getColumnIndex(ContactsContract.Contacts._ID);
			String contactId = cur.getString(idColumn);
			if (!IsContactBelongToAccount(contactId, sAccount)) {
				continue;
			}

			sXml.append("<Contact>");
			sXml.append(EAUtil.Cursor2Xml(cur));

			// 获得联系人的ID号
			sXml.append(GetEMail(contactId));
			sXml.append(GetPhones(contactId));

			sXml.append(GetNickName(contactId));
			sXml.append(GetNotes(contactId));

			sXml.append(GetOrg(contactId));
			sXml.append(GetAddr(contactId));
			sXml.append(GetIM(contactId));
			sXml.append(GetName(contactId));

			sXml.append(GetWebsite(contactId));
			sXml.append(GetBirthday(contactId));

			sXml.append("</Contact>");

		} while (cur.moveToNext());

		cur.close();

		sXml.append("</Contacts>");

		return sXml.toString();
	}

	static public ContactDetailInfo ContactParser(String sID,
			String sFamilyName, String sMiddleName, String sGivenName,
			String sDisplayName, String sPhone, String sAddr, String sOrg,
			String sCMail, String sNotes, String sIM, String sWebSite) {
		ContactDetailInfo c = new ContactDetailInfo();
		c.lID = Long.parseLong(sID);
		c.sFamilyName = sFamilyName.trim();
		c.sMiddleName = sMiddleName.trim();
		c.sGivenName = sGivenName.trim();
		c.sDisplayName = sDisplayName.trim();

		if (c.sDisplayName.length() < 1) {
			c.sDisplayName = c.sGivenName + " " + c.sMiddleName + " "
					+ c.sFamilyName;
		}

		if (sPhone != null && sPhone.length() > 0) {
			String[] sPhoneList = sPhone.split(";");
			for (int i = 0; i < sPhoneList.length; ++i) {
				String sPhoneItem = sPhoneList[i];
				if (sPhoneItem == null || sPhoneItem.length() <= 1) {
					continue;
				}

				String[] item = sPhoneItem.split(":");
				if (item.length != 2) {
					continue;
				}

				if (c.phoneList == null) {
					c.phoneList = new ArrayList<String>();
				}

				c.phoneList.add(sPhoneItem.trim());
			}
		}

		if (sAddr != null && sAddr.length() > 0) {
			String[] ssAddrList = sAddr.split(";;");
			for (int i = 0; i < ssAddrList.length; ++i) {
				String sAddrItem = ssAddrList[i];
				if (sAddrItem == null || sAddrItem.length() < 1) {
					continue;
				}

				String[] item = sAddrItem.split(":");
				if (item.length != 2) {
					continue;
				}

				if (c.AddrList == null) {
					c.AddrList = new ArrayList<String>();
				}

				c.AddrList.add(sAddrItem.trim());
			}
		}

		if (sCMail != null && sCMail.length() > 0) {
			String[] sMailList = sCMail.split(";");
			for (int i = 0; i < sMailList.length; ++i) {
				String sMailItem = sMailList[i];
				if (sMailItem == null || sMailItem.length() < 1) {
					continue;
				}

				String[] item = sMailItem.split(":");
				if (item.length != 2) {
					continue;
				}

				if (c.EMailList == null) {
					c.EMailList = new ArrayList<String>();
				}

				c.EMailList.add(sMailItem.trim());
			}
		}

		if (sWebSite != null && sWebSite.length() > 0) {
			String[] sSiteList = sWebSite.split("\\}");
			for (int i = 0; i < sSiteList.length; ++i) {
				String sSiteItem = sSiteList[i];
				if (sSiteItem == null || sSiteItem.length() < 1) {
					continue;
				}

				String[] item = sSiteItem.split("\\[");
				if (item.length != 2) {
					continue;
				}

				if (c.WebSiteList == null) {
					c.WebSiteList = new ArrayList<String>();
				}

				c.WebSiteList.add(sSiteItem.trim());
			}
		}

		/*
		 * if (sOrg != null && sOrg.length() > 0) {
		 * 
		 * }
		 * 
		 * if (sNotes != null && sNotes.length() > 0) {
		 * 
		 * }
		 * 
		 * if (sIM != null && sIM.length() > 0) {
		 * 
		 * }
		 */

		return c;
	}

	/**
	 * 新建联系人的接口
	 * 
	 * @param String
	 *            accountName，accountType 为账号名账号类型，一般为NULL
	 * @throws RemoteException
	 * @throws OperationApplicationException
	 */

	public static int insertContact(ContactDetailInfo c) {
		try {
			if (IsContactExist(c)) {
				return EADefine.EA_RET_ITEM_ALREADY_EXIST;
			}
			
			if (c.lID > 0){
				return EADefine.EA_RET_ITEM_NOT_EXIST;
			}

			ContentResolver resolver = EAUtil.GetContentResolver();
			ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();

			// insert new record
			ops.add(ContentProviderOperation
					.newInsert(ContactsContract.RawContacts.CONTENT_URI)
					.withValue(ContactsContract.RawContacts.ACCOUNT_TYPE, null)
					.withValue(ContactsContract.RawContacts.ACCOUNT_NAME, null)
					.withValue(
							ContactsContract.RawContacts.AGGREGATION_MODE,
							ContactsContract.RawContacts.AGGREGATION_MODE_DISABLED)
					.build());

			// update name
			ops.add(ContentProviderOperation
					.newInsert(ContactsContract.Data.CONTENT_URI)
					.withValueBackReference(
							ContactsContract.Data.RAW_CONTACT_ID, 0)
					.withValue(
							ContactsContract.Data.MIMETYPE,
							ContactsContract.CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE)
					.withValue(
							ContactsContract.CommonDataKinds.StructuredName.FAMILY_NAME,
							c.sFamilyName)
					.withValue(
							ContactsContract.CommonDataKinds.StructuredName.MIDDLE_NAME,
							c.sMiddleName)
					.withValue(
							ContactsContract.CommonDataKinds.StructuredName.GIVEN_NAME,
							c.sGivenName).build());

			// update phone
			if (c.phoneList != null) {
				for (int j = 0; j < c.phoneList.size(); j++) {
					String p = c.phoneList.get(j);
					String[] p1 = p.split(":");
					int iType = Integer.parseInt(p1[0]);

					ops.add(ContentProviderOperation
							.newInsert(ContactsContract.Data.CONTENT_URI)
							.withValueBackReference(
									ContactsContract.Data.RAW_CONTACT_ID, 0)
							.withValue(
									ContactsContract.Data.MIMETYPE,
									ContactsContract.CommonDataKinds.Phone.CONTENT_ITEM_TYPE)
							.withValue(
									ContactsContract.CommonDataKinds.Phone.NUMBER,
									p1[1]).withValue(Phone.TYPE, iType).build());
				}
			}

			// update email
			if (c.EMailList != null) {
				for (int j = 0; j < c.EMailList.size(); j++) {
					String p = c.EMailList.get(j);
					String[] p1 = p.split(":");
					if (p1.length != 2) {
						continue;
					}

					int iType = Integer.parseInt(p1[0]);
					String sEmail = p1[1];
					if (sEmail == "") {
						continue;
					}

					ops.add(ContentProviderOperation
							.newInsert(ContactsContract.Data.CONTENT_URI)
							.withValueBackReference(
									ContactsContract.Data.RAW_CONTACT_ID, 0)
							.withValue(
									ContactsContract.Data.MIMETYPE,
									ContactsContract.CommonDataKinds.Email.CONTENT_ITEM_TYPE)
							.withValue(
									ContactsContract.CommonDataKinds.Email.DATA,
									sEmail)
							.withValue(
									ContactsContract.CommonDataKinds.Email.TYPE,
									iType).build());
				}
			}

			// update website
			if (c.WebSiteList != null) {
				for (int j = 0; j < c.WebSiteList.size(); j++) {
					String p = c.WebSiteList.get(j);
					String[] p1 = p.split("\\[");
					if (p1.length != 2) {
						continue;
					}

					int iType = Integer.parseInt(p1[0]);
					String sURL = p1[1];
					if (sURL == "") {
						continue;
					}

					ops.add(ContentProviderOperation
							.newInsert(ContactsContract.Data.CONTENT_URI)
							.withValueBackReference(
									ContactsContract.Data.RAW_CONTACT_ID, 0)
							.withValue(
									ContactsContract.Data.MIMETYPE,
									ContactsContract.CommonDataKinds.Website.CONTENT_ITEM_TYPE)
							.withValue(
									ContactsContract.CommonDataKinds.Website.URL,
									sURL)
							.withValue(
									ContactsContract.CommonDataKinds.Website.TYPE,
									iType).build());
				}
			}

			// update address
			if (c.AddrList != null) {
				for (int j = 0; j < c.AddrList.size(); j++) {
					String[] a1 = c.AddrList.get(j).split(":");
					if (a1.length != 2) {
						continue;
					}

					int iType = Integer.parseInt(a1[0]);
					String sAddrItem = a1[1];
					if (sAddrItem.length() < 1) {
						continue;
					}

					String[] sAddrItems = sAddrItem.split(";");
					if (sAddrItems == null || sAddrItems.length < 1) {
						continue;
					}

					String street = "", city = "", region = "", postcode = "", country = "";
					for (int m = 0; m < sAddrItems.length; ++m) {
						String sAddr = sAddrItems[m];
						if (sAddr == null || sAddr.length() < 1) {
							continue;
						}

						String[] sAddrs = sAddr.split("=");
						if (sAddrs.length != 2) {
							// invalid addr
							continue;
						}

						if (sAddrs[0].equals(EADefine.EA_ADDR_STREET_TAG)) {
							street = sAddrs[1];
						} else if (sAddrs[0].equals(EADefine.EA_ADDR_CITY_TAG)) {
							city = sAddrs[1];
						} else if (sAddrs[0]
								.equals(EADefine.EA_ADDR_REGION_TAG)) {
							region = sAddrs[1];
						} else if (sAddrs[0]
								.equals(EADefine.EA_ADDR_POST_CODE_TAG)) {
							postcode = sAddrs[1];
						} else if (sAddrs[0]
								.equals(EADefine.EA_ADDR_COUNTRY_TAG)) {
							country = sAddrs[1];
						}
					}

					ops.add(ContentProviderOperation
							.newInsert(ContactsContract.Data.CONTENT_URI)
							.withValueBackReference(
									ContactsContract.Data.RAW_CONTACT_ID, 0)
							.withValue(
									ContactsContract.Data.MIMETYPE,
									ContactsContract.CommonDataKinds.StructuredPostal.CONTENT_ITEM_TYPE)
							.withValue(
									ContactsContract.CommonDataKinds.StructuredPostal.STREET,
									street)
							.withValue(
									ContactsContract.CommonDataKinds.StructuredPostal.CITY,
									city)
							.withValue(
									ContactsContract.CommonDataKinds.StructuredPostal.REGION,
									region)
							.withValue(
									ContactsContract.CommonDataKinds.StructuredPostal.POSTCODE,
									postcode)
							.withValue(
									ContactsContract.CommonDataKinds.StructuredPostal.COUNTRY,
									country)
							.withValue(
									ContactsContract.CommonDataKinds.StructuredPostal.TYPE,
									iType).build());

				}

			}

			/*
			 * if (c.ImList != null) { for (int j = 0; j < c.ImList.size(); j++)
			 * { String[] i1 = c.ImList.get(j).split(":"); String[] item = {
			 * i1[1] }; insertItemToContact(ops, i1[0], rawId,
			 * PROJECTION_IM_CONTACT, item); } } if (c.OrgList != null) { for
			 * (int j = 0; j < c.OrgList.size(); j++) { String[] o1 =
			 * c.OrgList.get(j).split(":"); String[] item = { o1[1] };
			 * insertItemToContact(ops, o1[0], rawId,
			 * PROJECTION_ORGANIZATION_CONTACT, item); } } if (c.NotesList !=
			 * null) { for (int j = 0; j < c.NotesList.size(); j++) { String[]
			 * n1 = c.NotesList.get(j).split(":"); String[] item = { n1[1] };
			 * insertItemToContact(ops, n1[0], rawId, PROJECTION_NOTES_CONTACT,
			 * item); } }
			 */

			ContentProviderResult[] cprs = resolver.applyBatch(
					ContactsContract.AUTHORITY, ops);
			for (int i = 0; i < cprs.length; ++i) {
				if (cprs[i].uri == null) {
					return EADefine.EA_RET_FAILED;
				}
			}

			return EADefine.EA_RET_OK;
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return EADefine.EA_RET_FAILED;
	}

	static public boolean delete(long lContactID) {
		if (0 == lContactID) {
			return false;
		}

		ArrayList<ContentProviderOperation> ops = new ArrayList<ContentProviderOperation>();
		ops.add(ContentProviderOperation
				.newDelete(
						ContentUris.withAppendedId(RawContacts.CONTENT_URI,
								lContactID)).build());

		try {
			ContentProviderResult[] cprs = EAUtil.GetContentResolver()
					.applyBatch(ContactsContract.AUTHORITY, ops);
			for (int i = 0; i < cprs.length; ++i) {
				if (cprs[i].count == 0) {
					return false;
				}
			}
		} catch (Exception e) {
			return false;
		}

		return true;
	}

	// update is implementd by insert a new contact and then remove the old one
	static public int update(ContactDetailInfo c) {
		try {
			int iRet = insertContact(c);
			if (EADefine.EA_RET_OK != iRet) {
				return iRet;
			}

			if (delete(c.lID)) {
				return EADefine.EA_RET_OK;
			}

			return EADefine.EA_RET_FAILED;

		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return EADefine.EA_RET_FAILED;
	}

	static public String GetContactGroupList() {
		ContentResolver resolver = EAUtil.GetContentResolver();
		// final String[] GROUP_PROJECTION = new String[] {
		// ContactsContract.Groups._ID, ContactsContract.Groups.TITLE };
		Cursor cursor = resolver.query(ContactsContract.Groups.CONTENT_URI,
				null, null, null, ContactsContract.Groups.TITLE + " ASC");
		if (cursor == null || !cursor.moveToFirst()) {
			if (cursor != null) {
				cursor.close();
			}
			return "";
		}

		StringBuilder sXml = new StringBuilder();
		sXml.append("<Groups>");

		do {
			sXml.append("<Group>");
			sXml.append(EAUtil.Cursor2Xml(cursor));
			sXml.append("</Group>");

		} while (cursor.moveToNext());

		sXml.append("</Groups>");

		return sXml.toString();
	}

}
