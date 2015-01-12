package mobitnt.net;

import java.io.BufferedReader;
import java.io.File;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.Properties;

import android.content.res.AssetManager;
import android.graphics.Bitmap;
import mobitnt.android.data.MimeTypes;
import mobitnt.android.wrapper.AppApi;
import mobitnt.android.wrapper.ContactApi;
import mobitnt.android.wrapper.MmsApi;
import mobitnt.util.*;

public class SrvSock extends NanoHTTPD {
	public final static int SERVER_PORT = 48300;
	public AssetManager htmlData = null;

	public SrvSock() throws IOException {
		super(SERVER_PORT);
	}

	public int ReadByteAry(BufferedReader in, byte[] data, int iCount) {
		int iLen = 0;
		while (iLen < iCount) {
			int iChar = -1;
			try {
				iChar = in.read();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			if (iChar == -1) {
				return iLen;
			}

			if (iChar > 255) {
				int temp = iChar;
				byte[] b = new byte[2];
				for (int i = b.length - 1; i > -1; i--) {
					b[i] = Integer.valueOf(temp & 0xff).byteValue();
					temp = temp >> 8;
				}

				data[iLen++] = b[0];
				data[iLen++] = b[1];
			} else {
				data[iLen++] = (byte) iChar;
			}

		}
		return iCount;
	}

	public String ReadLine(InputStream is) {
		int iReadByte = 0;
		int iPos = 0;

		byte[] httpData = new byte[EADefine.BUFF_SIZE];

		try {
			while ((iReadByte = is.read()) >= 0) {
				if (iPos >= EADefine.BUFF_SIZE) {
					// error but not handled
					// BUGTOBEFIX
					break;
				}

				httpData[iPos++] = (byte) iReadByte;
				if (iReadByte == '\n') {
					break;
				}
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		if (iPos <= 0) {
			return null;
		}

		byte[] stringbyte = new byte[iPos];
		for (int i = 0; i < iPos; ++i) {
			stringbyte[i] = httpData[i];
		}

		return new String(stringbyte);

	}

	public Response serve(String sUri, String method, Properties header,
			Properties parms, Properties files) {
		
		if (method.equalsIgnoreCase("POST")) {
			acquireWakeLock();
			String sAction = parms.getProperty("action", "");
			if (sAction.equalsIgnoreCase("installapk")) {
				String sPath = files.getProperty("filesavepath", "");
				AppApi.InstallApp(sPath);
				return new Response(HTTP_OK, MIME_HTML, "ok");
			}

			if (sAction.equalsIgnoreCase("uploadfile")) {
				String sFileSize = parms.getProperty("filesize", "0");
				long lFileSize = Long.parseLong(sFileSize);
				String sRet = "0";
				String sDstPath = files.getProperty("filesavepath", "");
				if (sDstPath.length() > 0) {
					File resFile = new File(sDstPath);
					if (resFile.exists()) {
						if (lFileSize == resFile.length()) {
							sRet = "1";
						} else {
							resFile.delete();
							return new Response(HTTP_INTERNALERROR, MIME_HTML,sRet);
						}
					}
				}

				return new Response(HTTP_OK, MIME_HTML, sRet);
			}
		}

		if (sUri.contains(".xml")) {
			String sRespType = MIME_XML;
			String sXmlMsg = AjaxRequestParser(sUri, parms, sRespType);
			Response r = new Response(HTTP_OK, sRespType, sXmlMsg);
			r.addHeader("Content-length", "" + sXmlMsg.getBytes().length);
			r.addHeader("Charset", "utf-8");
			MobiTNTLog.write("Waiting4Req");
			return r;
		}
		
		acquireWakeLock();

		String sAction = parms.getProperty("action", "n");
		if (sAction.contains("getfile")) {
			String sFile = parms.getProperty("file", "");

			// String sFile1 = java.net.URLDecoder.decode(sFile);
			
			return serveFileFromLocal(sUri, header, sFile);
		}

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_MMS_MIME_DATA)) {
			acquireWakeLock();
			String sMmsID = parms.getProperty(EADefine.EA_PART_ID_TAG, "0");
			String sType = parms.getProperty(EADefine.EA_MIME_TYPE_TAG, "text");

			Bitmap bm = MmsApi.getMmsImage(sMmsID);

			ByteArrayOutputStream out = new ByteArrayOutputStream();
			if (!bm.compress(Bitmap.CompressFormat.PNG, 100, out)) {
				return new Response(HTTP_OK, MIME_HTML, "error");
			}

			InputStream is = new ByteArrayInputStream(out.toByteArray());
			String mime = sType;

			// Support (simple) skipping:
			long startFrom = 0;
			// String range = header.getProperty("range");

			try {
				Response r = new Response(HTTP_OK, mime, is);
				r.addHeader("Content-length", "" + (is.available() - startFrom));
				r.addHeader(
						"Content-range",
						"" + startFrom + "-" + (is.available() - 1) + "/"
								+ is.available());
				return r;
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			return null;
		}

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_CONTACT_PHOTO)) {
			acquireWakeLock();
			
			String sPhotoID = parms.getProperty(EADefine.EA_PHOTO_ID_TAG, "0");
			Bitmap bm = ContactApi.GetPhoto(sPhotoID);

			ByteArrayOutputStream out = new ByteArrayOutputStream();
			if (bm == null || !bm.compress(Bitmap.CompressFormat.PNG, 100, out)) {
				return new Response(HTTP_OK, MIME_HTML, "error");
			}

			InputStream is = new ByteArrayInputStream(out.toByteArray());
			String mime = MIME_DEFAULT_BINARY;

			// Support (simple) skipping:
			long startFrom = 0;
			// String range = header.getProperty("range");

			try {
				Response r = new Response(HTTP_OK, mime, is);
				r.addHeader("Content-length", "" + (is.available() - startFrom));
				r.addHeader(
						"Content-range",
						"" + startFrom + "-" + (is.available() - 1) + "/"
								+ is.available());
				return r;
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return new Response(HTTP_OK, MIME_HTML, "error");
		}

		if (sAction.contains("getappicon")) {
		
			String sAppName = parms.getProperty("appname", "n");
			ByteArrayOutputStream out = AppApi.GetAppIcon(sAppName);
			if (out == null) {
				return new Response(HTTP_OK, MIME_HTML, "error");
			}

			InputStream is = new ByteArrayInputStream(out.toByteArray());
			String mime = "image/png";

			// Support (simple) skipping:
			long startFrom = 0;
			// String range = header.getProperty("range");

			try {
				Response r = new Response(HTTP_OK, mime, is);
				r.addHeader("Content-length", "" + (is.available() - startFrom));
				r.addHeader(
						"Content-range",
						"" + startFrom + "-" + (is.available() - 1) + "/"
								+ is.available());
				return r;
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return new Response(HTTP_OK, MIME_HTML, "error");
		}

		String sFileName = sUri;
		if (sUri.equals("/")) {
			sFileName = "/SendSms.html";
		}

		if (sFileName.equalsIgnoreCase("/SendSms.html")) {
			SmsManager.m_sSender = parms.getProperty("name", "");
			SmsManager.m_sPhone = parms.getProperty("phone", "");
		}

		MobiTNTLog.write("return file" + sFileName);
		return serveFile(sUri, header, sFileName);
	}

	public String AjaxRequestParser(String sRequest, Properties parms,
			String sRespType) {
		if (!sRequest.contains(EADefine.EA_ACT_QUERY_SYS_EVT)){
			acquireWakeLock();
		}
		
		/* here response to ajax request */
		PageGen pageGen = null;
		if (sRequest.contains("CallLog.xml")) {
			pageGen = new CallLogManager();
		} else if (sRequest.contains("SysInfo.xml")) {
			pageGen = new SysManager();
		} else if (sRequest.contains("SmsList.xml")) {
			pageGen = new SmsManager();
		} else if (sRequest.contains("ContactList.xml")) {
			pageGen = new ContactManager();
		} else if (sRequest.contains("Media.xml")) {
			pageGen = new MediaManager();
		} else if (sRequest.contains("AppList.xml")) {
			pageGen = new AppManager();
		} else if (sRequest.contains("FolderList.xml")) {
			pageGen = new StorageManager();
		} else if (sRequest.contains("AuthConn.xml")) {
			pageGen = new AuthManager();
		} else if (sRequest.contains("MmsList.xml")) {
			pageGen = new MmsManager();
		} else if (sRequest.contains("CalendarList.xml")) {
			pageGen = new CalendarManager();
		}

		if (pageGen == null) {
			return null;
		}

		// here assume all ajax request is from GET
		pageGen.m_iReqType = EADefine.HTTP_REQ_TYPE_GET;
		String sXmlResponse = pageGen.ProcessRequest(sRequest, parms);
		sRespType = pageGen.GetRespMimeType();
		if (sXmlResponse == null) {
			MobiTNTLog.write("failed to response request:" + sRequest);
			return PageGen.GenRetCode(EADefine.EA_RET_UNKONW_REQ);
		}
		return sXmlResponse;
	}

	InputStream GetFileFromAsset(String sFileName) throws IOException {
		if (sFileName.length() < 1) {
			return null;
		}

		if (!sFileName.startsWith("/", 0)) {
			sFileName = EADefine.EA_FILE_SEPERATOR + sFileName;
		}

		String sPath = EADefine.EA_HTML_PATH + sFileName;
		return htmlData.open(sPath);
	}

	/**
	 * Serves file from homeDir and its' subdirectories (only). Uses only URI,
	 * ignores all headers and HTTP parameters.
	 */
	public Response serveFile(String uri, Properties header, String sFileName) {
		// Remove URL arguments
		uri = uri.trim().replace(File.separatorChar, '/');
		if (uri.indexOf('?') >= 0)
			uri = uri.substring(0, uri.indexOf('?'));

		// Prohibit getting out of current directory
		if (uri.startsWith("..") || uri.endsWith("..")
				|| uri.indexOf("../") >= 0) {
			String sXmlMsg = PageGen.GenRetCode(EADefine.EA_RET_ACCESS_DENINED);
			Response r = new Response(HTTP_OK, MIME_XML, sXmlMsg);
			r.addHeader("Content-length", "" + sXmlMsg.getBytes().length);
			return r;
		}

		try {
			// Get MIME type from file name extension, if possible
			String mime = MimeTypes.GetMimeTypes(sFileName);

			// Support (simple) skipping:
			long startFrom = 0;
			String range = header.getProperty("range");
			if (range != null) {
				if (range.startsWith("bytes=")) {
					range = range.substring("bytes=".length());
					int minus = range.indexOf('-');
					if (minus > 0)
						range = range.substring(0, minus);
					try {
						startFrom = Long.parseLong(range);
					} catch (NumberFormatException nfe) {
					}
				}
			}

			// AssetFileDescriptor file = GetFileFromAsset(sFileName);
			InputStream fis = GetFileFromAsset(sFileName);

			fis.skip(startFrom);
			Response r = new Response(HTTP_OK, mime, fis);
			r.addHeader("Content-length", "" + (fis.available() - startFrom));
			r.addHeader(
					"Content-range",
					"" + startFrom + "-" + (fis.available() - 1) + "/"
							+ fis.available());
			return r;
		} catch (Exception e) {
			String sXmlMsg = PageGen
					.ReturnException("FORBIDDEN: Reading file failed:"
							+ e.toString());
			Response r = new Response(HTTP_OK, MIME_XML, sXmlMsg);
			r.addHeader("Content-length", "" + sXmlMsg.getBytes().length);
			return r;
			/*
			 * return new Response(HTTP_FORBIDDEN, MIME_PLAINTEXT,
			 * "FORBIDDEN: Reading file failed.");
			 */
		}
	}

	/**
	 * Serves file from homeDir and its' subdirectories (only). Uses only URI,
	 * ignores all headers and HTTP parameters.
	 */
	public Response serveFileFromLocal(String uri, Properties header,
			String sFileName) {
		// Remove URL arguments
		uri = uri.trim().replace(File.separatorChar, '/');
		if (uri.indexOf('?') >= 0)
			uri = uri.substring(0, uri.indexOf('?'));

		// Prohibit getting out of current directory
		if (uri.startsWith("..") || uri.endsWith("..")
				|| uri.indexOf("../") >= 0) {
			String sXmlMsg = PageGen.GenRetCode(EADefine.EA_RET_ACCESS_DENINED);
			Response r = new Response(HTTP_OK, MIME_XML, sXmlMsg);
			r.addHeader("Content-length", "" + sXmlMsg.getBytes().length);
			return r;
			/*
			 * return new Response(HTTP_FORBIDDEN, MIME_PLAINTEXT,
			 * "FORBIDDEN: Won't serve ../ for security reasons.");
			 */
		}

		try {
			// Get MIME type from file name extension, if possible
			String mime = MimeTypes.GetMimeTypes(sFileName);

			// Support (simple) skipping:
			long startFrom = 0;
			String range = header.getProperty("range");
			if (range != null) {
				if (range.startsWith("bytes=")) {
					range = range.substring("bytes=".length());
					int minus = range.indexOf('-');
					if (minus > 0)
						range = range.substring(0, minus);
					try {
						startFrom = Long.parseLong(range);
					} catch (NumberFormatException nfe) {
					}
				}
			}

			FileInputStream fis = new FileInputStream(sFileName);

			fis.skip(startFrom);
			Response r = new Response(HTTP_OK, mime, fis);
			int iPos = sFileName.lastIndexOf('/');
			String sDestFile = sFileName;
			try {

				if (iPos > 0 && iPos < sFileName.length()) {
					sDestFile = sFileName.substring(iPos + 1);
				}
			} catch (Exception e) {
				sDestFile = sFileName;
			}

			r.addHeader("Content-Disposition", "attachment; filename="
					+ sDestFile);
			r.addHeader("Content-length", "" + (fis.available() - startFrom));
			r.addHeader(
					"Content-range",
					"" + startFrom + "-" + (fis.available() - 1) + "/"
							+ fis.available());
			return r;
		} catch (Exception e) {
			String sXmlMsg = PageGen
					.ReturnException("FORBIDDEN: Reading file failed:"
							+ e.toString());
			Response r = new Response(HTTP_OK, MIME_XML, sXmlMsg);
			r.addHeader("Content-length", "" + sXmlMsg.getBytes().length);
			return r;
			/*
			 * return new Response(HTTP_FORBIDDEN, MIME_PLAINTEXT,
			 * "FORBIDDEN: Reading file failed.");
			 */
		}
	}

	public static InputStream newInputStream(final ByteBuffer buf) {
		return new InputStream() {
			public synchronized int read() throws IOException {
				if (!buf.hasRemaining()) {
					return -1;
				}
				return buf.get();
			}

			public synchronized int read(byte[] bytes, int off, int len)
					throws IOException {
				// Read only what's left
				len = Math.min(len, buf.remaining());
				buf.get(bytes, off, len);
				return len;
			}
		};
	}

}
