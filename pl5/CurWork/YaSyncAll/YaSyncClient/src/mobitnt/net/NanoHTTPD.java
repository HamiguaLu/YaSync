package mobitnt.net;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Date;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Locale;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.TimeZone;
import java.util.Vector;

import android.annotation.SuppressLint;
import android.content.Context;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

import mobitnt.util.EAUtil;
import mobitnt.util.MobiTNTLog;

public class NanoHTTPD {
	// ==================================================
	// API parts
	// ==================================================

	private static WakeLock sWakeLock = null;
	private static WifiLock sWifiLock = null;

	@SuppressLint("Wakelock")
	static public void acquireWakeLock() {
		if (sWakeLock == null) {
			PowerManager pMgr = (PowerManager) EAUtil.GetEAContext()
					.getSystemService(Context.POWER_SERVICE);

			sWakeLock = pMgr.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
					"YaSynccService.sync() wakelock.");

			WifiManager wMgr = (WifiManager) EAUtil.GetEAContext()
					.getSystemService(Context.WIFI_SERVICE);
			sWifiLock = wMgr.createWifiLock("YaSync network");
		}

		if (!sWakeLock.isHeld()) {
			sWakeLock.acquire(1000 * 45);
		}

		if (!sWifiLock.isHeld()) {
			sWifiLock.acquire();
		}
	}

	static public void releaseWakeLock() {
		/*
		 * if (sWakeLock != null) { if (sWakeLock.isHeld()){
		 * sWakeLock.release(); } }
		 */
		try {
			if (sWifiLock != null) {
				if (sWifiLock.isHeld()) {
					sWifiLock.release();
				}
			}
		} catch (Exception e) {
		}

	}

	public Response serve(String uri, String method, Properties header,
			Properties parms, Properties files) {
		return null;

	}

	/**
	 * Decodes the percent encoding scheme. <br/>
	 * For example: "an+example%20string" -> "an example string"
	 */
	public String decodePercent(String str) throws InterruptedException {
		try {
			return java.net.URLDecoder.decode(str);
		} catch (Exception e) {
			MobiTNTLog.write("decodePercent error:" + str);
			return "";
		}
	}

	/**
	 * Decodes parameters in percent-encoded URI-format ( e.g.
	 * "name=Jack%20Daniels&pass=Single%20Malt" ) and adds them to given
	 * Properties. NOTE: this doesn't support multiple identical keys due to the
	 * simplicity of Properties -- if you need multiples, you might want to
	 * replace the Properties with a Hastable of Vectors or such.
	 */
	public void decodeParms(String parms, Properties p)
			throws InterruptedException {
		if (parms == null)
			return;

		StringTokenizer st = new StringTokenizer(parms, "&");
		while (st.hasMoreTokens()) {
			String e = st.nextToken();
			int sep = e.indexOf('=');
			if (sep >= 0)
				p.put(decodePercent(e.substring(0, sep)).trim(),
						decodePercent(e.substring(sep + 1)));
			else
				p.put(decodePercent(e).trim(), "");
		}
	}

	/**
	 * HTTP response. Return one of these from serve().
	 */
	public class Response {
		/**
		 * Default constructor: response = HTTP_OK, data = mime = 'null'
		 */
		public Response() {
			this.status = HTTP_OK;
		}

		/**
		 * Basic constructor.
		 */
		public Response(String status, String mimeType, InputStream data) {
			this.status = status;
			this.mimeType = mimeType;
			this.data = data;
		}

		/**
		 * Convenience method that makes an InputStream out of given text.
		 */
		public Response(String status, String mimeType, String txt) {
			this.status = status;
			this.mimeType = mimeType;
			this.data = new ByteArrayInputStream(txt.getBytes());
		}

		/**
		 * Adds given line to the header.
		 */
		public void addHeader(String name, String value) {
			header.put(name, value);
		}

		/**
		 * HTTP status code after processing, e.g. "200 OK", HTTP_OK
		 */
		public String status;

		/**
		 * MIME type of content, e.g. "text/html"
		 */
		public String mimeType;

		/**
		 * Data of the response, may be null.
		 */
		public InputStream data;

		/**
		 * Headers for the HTTP response. Use addHeader() to add lines.
		 */
		public Properties header = new Properties();
	}

	/**
	 * Some HTTP response status codes
	 */
	public static final String HTTP_OK = "200 OK",
			HTTP_REDIRECT = "301 Moved Permanently",
			HTTP_FORBIDDEN = "403 Forbidden", HTTP_NOTFOUND = "404 Not Found",
			HTTP_BADREQUEST = "400 Bad Request",
			HTTP_INTERNALERROR = "500 Internal Server Error",
			HTTP_NOTIMPLEMENTED = "501 Not Implemented";

	/**
	 * Common mime types for dynamic content
	 */
	public static final String MIME_PLAINTEXT = "text/plain",
			MIME_HTML = "text/html", MIME_XML = "text/xml",
			MIME_DEFAULT_BINARY = "application/octet-stream";

	// ==================================================
	// Socket & server code
	// ==================================================

	/**
	 * Starts a HTTP server to given port.
	 * <p>
	 * Throws an IOException if the socket is already in use
	 */
	public NanoHTTPD(int port) throws IOException {
		myTcpPort = port;
		// ServerSocket a = new ServerSocket(myTcpPort);
		myServerSocket = new ServerSocket(myTcpPort);
		// myServerSocket = CreateSSlSrvSocket();
		myThread = new Thread(new Runnable() {
			public void run() {
				try {
					while (true)
						new HTTPSession(myServerSocket.accept());
				} catch (IOException ioe) {
				}
			}
		});
		myThread.setDaemon(true);
		myThread.start();
	}

	/**
	 * Stops the server.
	 */
	public void stop() {
		try {
			myServerSocket.close();
			myThread.join();
		} catch (IOException ioe) {
		} catch (InterruptedException e) {
		}
	}

	/**
	 * Handles one session, i.e. parses the HTTP request and returns the
	 * response.
	 */
	private class HTTPSession implements Runnable {
		private Socket mySocket;

		public HTTPSession(Socket s) {
			mySocket = s;
			Thread t = new Thread(this);
			t.setDaemon(true);
			t.start();
		}

		/**
		 * It returns the offset separating multipart file headers from the
		 * file's data.
		 **/
		private int stripMultipartHeaders(byte[] b, int offset) {
			int i = 0;
			for (i = offset; i < b.length; i++) {
				if (b[i] == '\r' && b[++i] == '\n' && b[++i] == '\r'
						&& b[++i] == '\n')
					break;
			}
			return i + 1;
		}

		/**
		 * Decodes the sent headers and loads the data into java Properties' key
		 * - value pairs
		 **/
		private void decodeHeader(BufferedReader in, Properties pre,
				Properties parms, Properties header)
				throws InterruptedException {
			try {
				// Read the request line
				String inLine = in.readLine();
				if (inLine == null)
					return;
				StringTokenizer st = new StringTokenizer(inLine);
				if (!st.hasMoreTokens())
					sendError(HTTP_BADREQUEST, "BAD REQUEST: Syntax error");

				String method = st.nextToken();
				pre.put("method", method);

				if (!st.hasMoreTokens())
					sendError(HTTP_BADREQUEST, "BAD REQUEST: Missing URI");

				String uri = st.nextToken();

				// Decode parameters from the URI
				int qmi = uri.indexOf('?');
				if (qmi >= 0) {
					decodeParms(uri.substring(qmi + 1), parms);
					uri = decodePercent(uri.substring(0, qmi));
				} else
					uri = decodePercent(uri);

				// If there's another token, it's protocol version,
				// followed by HTTP headers. Ignore version but parse headers.
				// NOTE: this now forces header names lowercase since they are
				// case insensitive and vary by client.
				if (st.hasMoreTokens()) {
					String line = in.readLine();
					while (line != null && line.trim().length() > 0) {
						int p = line.indexOf(':');
						if (p >= 0)
							header.put(
									line.substring(0, p).trim()
											.toLowerCase(Locale.ENGLISH), line
											.substring(p + 1).trim());
						line = in.readLine();
					}
				}

				pre.put("uri", uri);
			} catch (IOException ioe) {
				sendError(
						HTTP_INTERNALERROR,
						"SERVER INTERNAL ERROR: IOException: "
								+ ioe.getMessage());
			}
		}

		public int saveFileFromPost(byte[] b, int offset, int len,
				String szFileName, boolean bAppend) {
			if (len <= 0) {
				return -1;
			}

			try {
				File temp = new File(szFileName);
				OutputStream fstream = new FileOutputStream(temp, bAppend);
				fstream.write(b, offset, len);
				fstream.flush();
				fstream.close();
				return 0;
			} catch (Exception e) { // Catch exception if any

			}

			return -1;
		}

		/**
		 * Decodes the Multipart Body data and put it into java Properties' key
		 * - value pairs.
		 **/
		@SuppressLint("DefaultLocale")
		private void decodeMultipartData(InputStream is, long size,
				String boundary, byte[] fbuf, BufferedReader in,
				Properties parms, Properties files) throws InterruptedException {
			try {
				int[] bpositions = getBoundaryPositions(fbuf,
						boundary.getBytes());
				int boundarycount = 1;
				String mpline = in.readLine();
				while (mpline != null) {
					if (mpline.indexOf(boundary) == -1)
						sendError(
								HTTP_BADREQUEST,
								"BAD REQUEST: Content type is multipart/form-data but next chunk does not start with boundary");
					boundarycount++;
					Properties item = new Properties();
					mpline = in.readLine();
					while (mpline != null && mpline.trim().length() > 0) {
						int p = mpline.indexOf(':');
						if (p != -1)
							item.put(
									mpline.substring(0, p).trim()
											.toLowerCase(Locale.ENGLISH),
									mpline.substring(p + 1).trim());
						mpline = in.readLine();
					}

					if (mpline == null) {
						break;
					}

					String contentDisposition = item
							.getProperty("content-disposition");
					if (contentDisposition == null) {
						sendError(
								HTTP_BADREQUEST,
								"BAD REQUEST: Content type is multipart/form-data but no content-disposition info found");
					}
					StringTokenizer st = new StringTokenizer(
							contentDisposition, "; ");
					Properties disposition = new Properties();
					while (st.hasMoreTokens()) {
						String token = st.nextToken();
						int p = token.indexOf('=');
						if (p != -1)
							disposition.put(token.substring(0, p).trim()
									.toLowerCase(), token.substring(p + 1)
									.trim());
					}

					String pname = disposition.getProperty("name");
					pname = pname.substring(1, pname.length() - 1);

					String value = "";
					if (item.getProperty("content-type") == null) {
						while (mpline != null && mpline.indexOf(boundary) == -1) {
							mpline = in.readLine();
							if (mpline != null) {
								int d = mpline.indexOf(boundary);
								if (d == -1)
									value += mpline;
								else
									value += mpline.substring(0, d - 2);
							}
						}
						parms.put(pname, value);
						continue;
					}

					/*
					 * if (boundarycount > bpositions.length)
					 * sendError(HTTP_INTERNALERROR,
					 * "Error processing request");
					 */
					int offset = stripMultipartHeaders(fbuf,
							bpositions[boundarycount - 2]);
					value = disposition.getProperty("filename");
					value = value.substring(1, value.length() - 1);

					String sDstPath = parms.getProperty("currentfolder", "");

					if (sDstPath.length() <= 0) {
						sDstPath = EAUtil.GetPEFolderOnSDCard();
						sDstPath += "/" + value;
					}

					int iRet = 0;
					if (size <= 0) {
						iRet = saveFileFromPost(fbuf, offset,
								bpositions[boundarycount - 1] - offset - 4,
								sDstPath, false);
						files.put("filesaveresult", String.valueOf(iRet));
						files.put("filesavepath", sDstPath);

						do {
							mpline = in.readLine();
						} while (mpline != null
								&& mpline.indexOf(boundary) == -1);

						parms.put(pname, value);
						continue;
					}

					int iLen = fbuf.length - offset;
					iRet = saveFileFromPost(fbuf, offset, iLen, sDstPath, false);
					int[] bEndPos = null;
					if (iRet == 0) {
						iRet = savePostData2File(is, size - 1024, sDstPath);
						if (iRet == 0) {
							byte[] bEndData = new byte[1024];
							if (is.read(bEndData, 0, 1024) != 1024) {
								iRet = 2;
							} else {
								bEndPos = getBoundaryPositions(bEndData,
										boundary.getBytes());
								if (bEndPos != null && bEndPos.length > 0) {
									iRet = saveFileFromPost(bEndData, 0,
											bEndPos[0] - 4, sDstPath, true);
								}
							}
						}
						/* Process end boundary */
					}

					files.put("filesaveresult", String.valueOf(iRet));
					files.put("filesavepath", sDstPath);
					break;
				}
			} catch (IOException ioe) {
				sendError(
						HTTP_INTERNALERROR,
						"SERVER INTERNAL ERROR: IOException: "
								+ ioe.getMessage());
			}
		}

		/**
		 * Find byte index separating header from body. It must be the last byte
		 * of the first two sequential new lines.
		 **/
		private int findHeaderEnd(final byte[] buf, int rlen) {
			int splitbyte = 0;
			while (splitbyte + 3 < rlen) {
				if (buf[splitbyte] == '\r' && buf[splitbyte + 1] == '\n'
						&& buf[splitbyte + 2] == '\r'
						&& buf[splitbyte + 3] == '\n')
					return splitbyte + 4;
				splitbyte++;
			}
			return 0;
		}

		/**
		 * Find the byte positions where multipart boundaries start.
		 **/
		public int[] getBoundaryPositions(byte[] b, byte[] boundary) {
			int matchcount = 0;
			int matchbyte = -1;
			Vector<Integer> matchbytes = new Vector<Integer>();
			for (int i = 0; i < b.length; i++) {
				if (b[i] == boundary[matchcount]) {
					if (matchcount == 0)
						matchbyte = i;
					matchcount++;
					if (matchcount == boundary.length) {
						matchbytes.addElement(Integer.valueOf(matchbyte));
						matchcount = 0;
						matchbyte = -1;
					}
				} else {
					i -= matchcount;
					matchcount = 0;
					matchbyte = -1;
				}
			}
			int[] ret = new int[matchbytes.size()];
			for (int i = 0; i < ret.length; i++) {
				ret[i] = ((Integer) matchbytes.elementAt(i)).intValue();
			}
			return ret;
		}

		public int savePostData2File(InputStream is, long size, String sDstPath) {
			try {
				File temp = new File(sDstPath);
				OutputStream fstream = new FileOutputStream(temp, true);

				byte[] buf = new byte[512];
				int rlen = 0;
				while (rlen >= 0 && size > 0) {
					long iRead = 512;
					if (size < 512) {
						iRead = size;
					}
					rlen = is.read(buf, 0, (int) iRead);
					if (rlen > 0) {
						fstream.write(buf, 0, rlen);
						size -= rlen;
					}
				}
				fstream.flush();
				fstream.close();
				return 0;
			} catch (Exception e) { // Catch exception if any

			}

			return -1;
		}

		public void run() {
			try {
				InputStream is = mySocket.getInputStream();
				if (is == null)
					return;

				// Read the first 8192 bytes.
				// The full header should fit in here.
				// Apache's default header limit is 8KB.
				// Do NOT assume that a single read will get the entire header
				// at once!
				final int bufsize = 8192;
				byte[] buf = new byte[bufsize];
				int splitbyte = 0;
				int rlen = 0;
				{
					int read = is.read(buf, 0, bufsize);
					if (read < 0) {
						return;
					}

					while (read > 0) {
						rlen += read;
						splitbyte = findHeaderEnd(buf, rlen);
						if (splitbyte > 0)
							break;
						read = is.read(buf, rlen, bufsize - rlen);
					}
				}

				// Create a BufferedReader for parsing the header.
				ByteArrayInputStream hbis = new ByteArrayInputStream(buf, 0,
						rlen);
				BufferedReader hin = new BufferedReader(new InputStreamReader(
						hbis));
				Properties pre = new Properties();
				Properties parms = new Properties();
				Properties header = new Properties();
				Properties files = new Properties();

				// Decode the header into parms and header java properties
				decodeHeader(hin, pre, parms, header);
				String method = pre.getProperty("method");
				String uri = pre.getProperty("uri");

				long size = 0x7FFFFFFFFFFFFFFFl;
				String contentLength = header.getProperty("content-length");
				if (contentLength != null) {
					try {
						size = Integer.parseInt(contentLength);
					} catch (NumberFormatException ex) {
					}
				}

				// Write the part of body already read to ByteArrayOutputStream
				// f
				ByteArrayOutputStream f = new ByteArrayOutputStream();
				if (splitbyte < rlen)
					f.write(buf, splitbyte, rlen - splitbyte);

				if (splitbyte < rlen)
					size -= rlen - splitbyte + 1;
				else if (splitbyte == 0 || size == 0x7FFFFFFFFFFFFFFFl)
					size = 0;

				// Now read all the body and write it to f
				buf = new byte[512];
				int iTotal = 0;
				while (rlen >= 0 && size > 0) {
					rlen = is.read(buf, 0, 512);
					size -= rlen;
					if (rlen > 0) {
						f.write(buf, 0, rlen);
						iTotal += rlen;
					}

					if (iTotal > 1024 * 5 && size > 1024) {
						break;
					}
				}

				// Get the raw body as a byte []
				byte[] fbuf = f.toByteArray();

				// Create a BufferedReader for easily reading it as string.
				ByteArrayInputStream bin = new ByteArrayInputStream(fbuf);
				BufferedReader in = new BufferedReader(new InputStreamReader(
						bin));

				// If the method is POST, there may be parameters
				// in data section, too, read it:
				if (method.equalsIgnoreCase("POST")) {
					String contentType = "";
					String contentTypeHeader = header
							.getProperty("content-type");
					StringTokenizer st = new StringTokenizer(contentTypeHeader,
							"; ");
					if (st.hasMoreTokens()) {
						contentType = st.nextToken();
					}

					if (contentType.equalsIgnoreCase("multipart/form-data")) {
						// Handle multipart/form-data
						if (!st.hasMoreTokens())
							sendError(HTTP_BADREQUEST,
									"BAD REQUEST: Content type is multipart/form-data but boundary missing");
						String boundaryExp = st.nextToken();
						st = new StringTokenizer(boundaryExp, "=");
						if (st.countTokens() != 2)
							sendError(HTTP_BADREQUEST,
									"BAD REQUEST: Content type is multipart/form-data but boundary syntax error");
						st.nextToken();
						String boundary = st.nextToken();

						decodeMultipartData(is, size, boundary, fbuf, in,
								parms, files);

					} else {
						// Handle application/x-www-form-urlencoded
						String postLine = "";
						char pbuf[] = new char[512];
						int read = in.read(pbuf);
						while (read >= 0 && !postLine.endsWith("\r\n")) {
							postLine += String.valueOf(pbuf, 0, read);
							read = in.read(pbuf);
						}
						postLine = postLine.trim();
						decodeParms(postLine, parms);
					}
				}

				/*
				 * if (method.equalsIgnoreCase("PUT")) files.put("content",
				 * saveTmpFile(fbuf, 0, f.size()));
				 */

				// Ok, now do the serve()
				Response r = serve(uri, method, header, parms, files);
				if (r == null)
					sendError(HTTP_INTERNALERROR,
							"SERVER INTERNAL ERROR: Serve() returned a null response.");
				else
					sendResponse(r.status, r.mimeType, r.header, r.data);

				in.close();
				is.close();

				releaseWakeLock();
			} catch (IOException ioe) {
				try {
					sendError(
							HTTP_INTERNALERROR,
							"SERVER INTERNAL ERROR: IOException: "
									+ ioe.getMessage());
				} catch (Throwable t) {
				}
			} catch (InterruptedException ie) {
				// Thrown by sendError, ignore and exit the thread.
			}
		}

		/**
		 * Returns an error message as a HTTP response and throws
		 * InterruptedException to stop furhter request processing.
		 */
		private void sendError(String status, String msg)
				throws InterruptedException {
			sendResponse(status, MIME_PLAINTEXT, null,
					new ByteArrayInputStream(msg.getBytes()));
			throw new InterruptedException();
		}

		/**
		 * Sends given response to the socket.
		 */
		private void sendResponse(String status, String mime,
				Properties header, InputStream data) {
			try {
				if (status == null)
					throw new Error("sendResponse(): Status can't be null.");

				OutputStream out = mySocket.getOutputStream();
				PrintWriter pw = new PrintWriter(out);
				pw.print("HTTP/1.0 " + status + " \r\n");

				if (mime != null)
					pw.print("Content-Type: " + mime + "\r\n");

				if (header == null || header.getProperty("Date") == null)
					pw.print("Date: " + gmtFrmt.format(new Date()) + "\r\n");

				if (header != null) {
					Enumeration<?> e = header.keys();
					while (e.hasMoreElements()) {
						String key = (String) e.nextElement();
						String value = header.getProperty(key);
						pw.print(key + ": " + value + "\r\n");
					}
				}

				pw.print("\r\n");
				pw.flush();

				if (data != null) {
					byte[] buff = new byte[2048];
					while (true) {
						int read = data.read(buff, 0, 2048);
						if (read <= 0)
							break;
						out.write(buff, 0, read);
					}
				}
				out.flush();
				out.close();
				if (data != null)
					data.close();
			} catch (IOException ioe) {
				// Couldn't write? No can do.
				try {
					mySocket.close();
				} catch (Throwable t) {
				}
			}
		}

	};

	private int myTcpPort;
	private final ServerSocket myServerSocket;
	private Thread myThread;
	File myFileDir;

	// ==================================================
	// File server code
	// ==================================================

	/**
	 * Serves file from homeDir and its' subdirectories (only). Uses only URI,
	 * ignores all headers and HTTP parameters.
	 */
	public Response serveFile(String uri, Properties header, File homeDir,
			boolean allowDirectoryListing) {
		return null;
	}

	/**
	 * Hashtable mapping (String)FILENAME_EXTENSION -> (String)MIME_TYPE
	 */

	private static Hashtable<String, String> theMimeTypes = new Hashtable<String, String>();
	static {
		StringTokenizer st = new StringTokenizer("htm		text/html "
				+ "html		text/html " + "txt		text/plain " + "asc		text/plain "
				+ "gif		image/gif " + "jpg		image/jpeg " + "jpeg		image/jpeg "
				+ "png		image/png " + "mp3		audio/mpeg "
				+ "m3u		audio/mpeg-url " + "pdf		application/pdf "
				+ "doc		application/msword " + "ogg		application/x-ogg "
				+ "zip		application/octet-stream "
				+ "exe		application/octet-stream "
				+ "class		application/octet-stream ");
		while (st.hasMoreTokens())
			theMimeTypes.put(st.nextToken(), st.nextToken());
	}

	/**
	 * GMT date formatter
	 */
	private static java.text.SimpleDateFormat gmtFrmt;
	static {
		gmtFrmt = new java.text.SimpleDateFormat(
				"E, d MMM yyyy HH:mm:ss 'GMT'", Locale.US);
		gmtFrmt.setTimeZone(TimeZone.getTimeZone("GMT"));
	}

}
