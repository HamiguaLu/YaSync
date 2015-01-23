package mobitnt.util;

import java.io.File;

public class EADefine {
	public static final int HTTP_REQ_TYPE_UNKNOW = 0;
	public static final int HTTP_REQ_TYPE_GET = 1;
	public static final int HTTP_REQ_TYPE_POST = 2;
	public static final String EA_HTML_PATH = "html";
	public static final String EA_LOG_FILE = "ealog.log";
	public static final int BUFF_SIZE = 1024; // 1k Byte
	public static final char EA_FILE_SEPERATOR = File.separatorChar;

	public static final String ValDelimiter = "&ensp;";
	public static final String EA_PREF_NAME = "PePref";
	public static final String PE_APP_VER = "24";

	// ////////////////////////////////////////////////////////////////
	// Error Code list
	public static final int EA_RET_OK = 0;

	public static final int EA_RET_FAILED = 1;
	public static final int EA_RET_AUTH_FAILED = 4;
	public static final int EA_RET_INVALID_SMS_CONTENT = 5;
	public static final int EA_RET_INVALID_PHONE_NO = 6;
	public static final int EA_RET_END_OF_FILE = 7;
	public static final int EA_RET_UNKONW_REQ = 8;
	public static final int EA_RET_INVALID_EMAIL = 9;
	public static final int EA_RET_ACCESS_DENINED = 11;
	public static final int EA_RET_QUERY_STATE_LATER = 12;
	public static final int EA_RET_SAVE_FILE_FAILED = 13;
	public static final int EA_RET_NEED_OP_ON_PHONE = 14;
	public static final int EA_RET_NO_EXTERNAL_STOREAGE = 15;
	public static final int EA_RET_NOT_SUPPORTED = 17;
	public static final int EA_RET_ITEM_ALREADY_EXIST = 18;
	public static final int EA_RET_INVALID_PARA = 19;
	public static final int EA_RET_CAN_NOT_CREATE_DEFAULT_CALENDAR = 20;
	public static final int EA_RET_ITEM_NOT_EXIST = 21;

	// /////////////////////////////////////////////////////////////////

	public static final int EA_RESPONSE_LIST_SIZE = 5;

	// Contact action
	public static final String EA_ACT_GET_CONTACT_LIST = "getcontactlist";
	public static final String EA_ACT_GET_CONTACT_DETAIL = "getcontactcetail";
	public static final String EA_ACT_UPDATE_CONTACT = "updatecontact";
	public static final String EA_ACT_INSERT_CONTACT = "insertcontact";
	public static final String EA_ACT_DELETE_CONTACT = "deletecontact";
	public static final String EA_ACT_GET_MAX_CONTACT_ID = "getmaxcontactid";
	public static final String EA_ACT_CHECK_NEW_CONTACT = "checknewcontact";
	public static final String EA_ACT_GET_CONTACT_COUNT = "getcontactcount";
	public static final String EA_ACT_GET_CONTACT_PHOTO = "getcontactphoto";
	public static final String EA_ACT_GET_CONTACT_GROUP_LIST = "getcontactgrouplist";

	// Sms action
	public static final String EA_ACT_SEND_SMS = "sendsms";
	public static final String EA_ACT_DELETE_SMS = "deletesms";
	public static final String EA_ACT_GET_CHAT = "getchat";
	public static final String EA_ACT_GET_THREAD_COUNT = "getthreadcount";
	public static final String EA_ACT_GET_SEND_STATE = "getsendstate";
	public static final String EA_ACT_GET_THREAD_LIST = "getthreadlist";
	public static final String EA_ACT_GET_SMS_LIST = "getsmslist";
	public static final String EA_ACT_RESTORE_SMS = "restoresms";
	public static final String EA_ACT_GET_SMS_COUNT = "getsmscount";
	public static final String EA_ACT_GET_SCHEDULE_SMS_LIST = "getschedulesmslist";
	public static final String EA_ACT_DEL_SCHEDULE_SMS = "delschedulesmslist";
	public static final String EA_ACT_ADD_SCHEDULE_SMS = "addschedulesms";
	public static final String EA_ACT_CHECK_NEW_SMS = "checknewsms";
	public static final String EA_ACT_GET_MAX_SMS_ID = "getmaxsmsid";

	// app action
	public static final String EA_ACT_GET_APP_LIST = "getapplist";
	public static final String EA_ACT_REFRESH_APP_LIST = "refreshapplist";
	public static final String EA_ACT_REMOVE_APP = "removeapp";
	public static final String EA_ACT_INSTALL_APP = "installapp";
	public static final String EA_ACT_UPDATE_APP_LIST = "updateapplist";
	public static final String EA_ACT_GET_APK_STORE_PATH = "getapkstorepath";

	// call action
	public static final String EA_ACT_RESTORE_CALL = "restorecall";
	public static final String EA_ACT_GET_CALL_COUNT = "getcalllogcount";
	public static final String EA_ACT_GET_CALL_LIST = "getcallloglist";
	public static final String EA_ACT_DELETE_CALL = "deletecalllog";
	public static final String EA_ACT_CHECK_NEW_CALL = "checknewcall";
	public static final String EA_ACT_GET_MAX_CALL_ID = "getmaxcallid";

	// media action
	public static final String EA_ACT_GET_IMAGE_COUNT = "getimagecount";
	public static final String EA_ACT_GET_VIDEO_COUNT = "getvideocount";
	public static final String EA_ACT_GET_AUDIO_COUNT = "getaudiocount";
	public static final String EA_ACT_GET_IMAGE_LIST = "getimagelist";
	public static final String EA_ACT_GET_VIDEO_LIST = "getvideolist";
	public static final String EA_ACT_GET_AUDIO_LIST = "getaudiolist";

	// storage action
	public static final String EA_ACT_GET_FOLDER_LIST = "getfolderlist";
	public static final String EA_ACT_REMOVE_FILE = "removefile";
	public static final String EA_ACT_CREATE_FILE = "createfile";
	public static final String EA_ACT_RENAME_FILE = "rename";
	public static final String EA_ACT_GET_EXTERNAL_ROOT_FOLDER = "getexternalrootfolder";
	public static final String EA_ACT_GET_SDCARD_LIST = "getsdcardlist";

	// calendar action
	public static final String EA_ACT_GET_CALENDAR_LIST = "getcalendarlist";
	public static final String EA_ACT_ADD_CALENDAR_EVENT = "addcalendarevent";
	public static final String EA_ACT_GET_CALENDAR_EVENT = "getcalendarevent";
	public static final String EA_ACT_DEL_CALENDAR_EVENT = "delcalendarevent";
	public static final String EA_ACT_GET_MAX_CAL_EVT_ID = "getmaxcalendarevtid";

	// sys action
	public static final String EA_ACT_QUERY_SYS_EVT = "queryevt";
	public static final String EA_ACT_GET_SYS_INFO = "getsysinfo";
	public static final String EA_ACT_GET_ACCOUNT_LIST = "getaccounts";
	public static final String EA_ACT_REQ_AUTH = "RequestAuth";

	// Mms action
	public static final String EA_ACT_GET_MAX_MMS_ID = "getmaxmmsid";
	public static final String EA_ACT_GET_MMS_LIST = "getmmslist";
	public static final String EA_ACT_DEL_MMS = "delmms";
	public static final String EA_ACT_GET_MMS_MIME_DATA = "getmmsmimedata";

	// following is TAG list
	//action tag
	public static final String EA_ACTION_TAG = "action";
	public static final String EA_FROM_TAG = "from";
	public static final String EA_TO_TAG = "to";
	public static final String EA_START_DATE_TAG = "startdate";
	
	//app tag
	public static final String EA_APP_NAME_TAG = "appname";
	public static final String EA_APK_STORE_PATH_TAG = "apkstorepath";
	public static final String EA_APK_PATH_TAG = "apkpath";

	// sms tag
	public static final String EA_THREAD_ID_TAG = "threadid";
	public static final String EA_SMS_CONTENT_TAG = "content";
	public static final String EA_SMS_RECEIVER_TAG = "receiver";
	public static final String EA_SMS_SCHEDULE_TIME_TAG = "smsscheduletime";
	public static final String EA_SMS_TIMESTAMP_TAG = "SmsTimestamp";
	public static final String EA_SMS_SEND_STATUS_TAG = "SmsSendStatus";
	public static final String EA_PART_ID_TAG = "partid";

	// calendar tag
	public static final String EA_CALENDAR_ID_TAG = "calendarid";
	public static final String EA_CALENDAR_EVT_ID_TAG = "eventid";
	public static final String EA_CALENDAR_TITLE_TAG = "title";
	public static final String EA_CALENDAR_DESC_TAG = "desc";
	public static final String EA_CALENDAR_TIMEZONE_TAG = "timezone";
	public static final String EA_CALENDAR_LOCATION_TAG = "location";
	public static final String EA_CALENDAR_RRULE_TAG = "rrule";
	public static final String EA_CALENDAR_STARTTIME_TAG = "starttime";
	public static final String EA_CALENDAR_ENDTIME_TAG = "endtime";
	public static final String EA_CALENDAR_DURATION_TAG = "duration";
	public static final String EA_CALENDAR_ALLDAY_TAG = "allday";
	public static final String EA_CALENDAR_HASALARM_TAG = "hasalarm";
	public static final String EA_CALENDAR_REMINDER_DELTA_TAG = "reminderdelta";

	// file tag
	public static final String EA_FILE_TAG = "file";
	public static final String EA_FILE_LIST_TAG = "filelist";
	//public static final String EA_IS_REAL_PATH_TAG = "isRealPath";
	public static final String EA_NEW_PATH_TAG = "newpath";

	public static final String EA_ID_TAG = "id";

	//contact tag
	public static final String EA_CONTACTID_TAG = "contactID";
	public static final String EA_ACCOUNT_NAME_TAG = "accountname";
	public static final String EA_CONTACT_MAIL_TAG = "cmail";
	public static final String EA_FAMILY_NAME_TAG = "familyname";
	public static final String EA_MIDDLE_NAME_TAG = "middlename";
	public static final String EA_GIVEN_NAME_TAG = "givenname";
	public static final String EA_DISPLAY_NAME_TAG = "displayname";
	public static final String EA_CONTACT_PHONE_TAG = "cphone";
	public static final String EA_CONTACT_ORG_TAG = "cOrg";
	public static final String EA_CONTACT_ADDR_TAG = "cAddr";
	public static final String EA_CONTACT_NOTES_TAG = "cNotes";
	public static final String EA_CONTACT_WEBSITE_TAG = "csite";
	public static final String EA_CONTACT_IM_TAG = "im";
	public static final String EA_PHOTO_ID_TAG = "photoid";

	// address tag
	public static final String EA_ADDR_CITY_TAG = "city";
	public static final String EA_ADDR_FORMAT_ADDR_TAG = "formataddr";
	public static final String EA_ADDR_POST_CODE_TAG = "postcode";
	public static final String EA_ADDR_REGION_TAG = "region";
	public static final String EA_ADDR_STREET_TAG = "street";
	public static final String EA_ADDR_COUNTRY_TAG = "country";

	//SYS tag
	public static final String EA_SYS_PRODUCT_MODEL_TAG = "ProductModel";
	public static final String EA_SYS_PRODUCT_MANUFACTURER_TAG = "ProductManufacturer";
	public static final String EA_SYS_EXT_AVAILABLE_SPACE = "SDCardAvailableSpace";
	public static final String EA_SYS_EXT_TOTAL_SPACE = "SDCardTotalSpace";
	public static final String EA_SYS_AVAIL_RAM = "AvailRAM";
	public static final String EA_SYS_TOTAL_RAM = "TotalRAM";
	public static final String EA_SYS_BATTERY_TAG = "BatteryLevel";
	public static final String EA_SYS_CPU_FREQ_TAG = "CpuFreq";
	public static final String EA_SYS_CPU_MODEL_TAG = "CpuModel";

	//misc tag
	public static final String EA_MIME_TYPE_TAG = "mimetype";
	public static final String EA_SECURITY_CODE_TAG = "securitycode";
	public static final String EA_PC_PORT_TAG = "pcport";
	
	//sys evt
	public final static int SYS_EVT_NONE = 100;
	public final static int SYS_EVT_SMS_CHANGED = 101;
	public final static int SYS_EVT_CALL_LOG_CHANGED = 102;
	public final static int SYS_EVT_CONTACT_CHANGED = 103;
	public final static int SYS_EVT_CALENDAR_CHANGED = 104;
	public final static int SYS_EVT_SMS_SENT_STATUS = 105;
	public final static int SYS_EVT_SMS_DELIVER_STATUS = 106;
	public final static int SYS_EVT_BATTERY_LEVEL_CHANGED = 107;
	
	//
	public final static String INTENT_ACTION_PHONE_STATE = "android.intent.action.PHONE_STATE";
	
	

}
