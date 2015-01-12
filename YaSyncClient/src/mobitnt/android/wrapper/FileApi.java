package mobitnt.android.wrapper;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;

import mobitnt.android.data.*;
import mobitnt.util.EAUtil;



public class FileApi {
	static public final int EA_FILE_TYPE_UNKNOW = 0;
	static public final int EA_FILE_TYPE_TXT = 1;
	static public final int EA_FILE_TYPE_AUDIO = 2;
	static public final int EA_FILE_TYPE_IMAGE = 3;
	static public final int EA_FILE_TYPE_VIDEO = 4;
	static public final int EA_FILE_TYPE_DIR = 5;
	static public final int EA_FILE_TYPE_ZIP = 6;
	static public final int EA_FILE_TYPE_APP = 7;
	static public final int EA_FILE_TYPE_APK = 8;
	static public final int MAX_FILE_COUNT	= 30;
	static public int GetFileCountInFolder(String sFolderPath) {
		File file = new File(sFolderPath);

		File[] files = file.listFiles();
		if (files == null){
			return 0;
		}
		
		return files.length;
	}
	
	public static String GetRealPath(String sVirtualPath){
		String sExternalRoot = FileApi.getExternalStoragePath();
		sVirtualPath = sExternalRoot + sVirtualPath;
		sVirtualPath = sVirtualPath.replace("//", "/");
		return sVirtualPath;
	}
	
	public static HashSet<String> getExternalMounts() {
	    final HashSet<String> out = new HashSet<String>();
	    String reg = "(?i).*vold.*(vfat|ntfs|exfat|fat32|ext3|ext4).*rw.*";
	    String s = "";
	    try {
	        final Process process = new ProcessBuilder().command("mount")
	                .redirectErrorStream(true).start();
	        process.waitFor();
	        final InputStream is = process.getInputStream();
	        final byte[] buffer = new byte[1024];
	        while (is.read(buffer) != -1) {
	            s = s + new String(buffer);
	        }
	        is.close();
	    } catch (final Exception e) {
	        e.printStackTrace();
	    }

	    // parse output
	    final String[] lines = s.split("\n");
	    for (String line : lines) {
	        if (!line.toLowerCase(Locale.US).contains("asec")) {
	            if (line.matches(reg)) {
	                String[] parts = line.split(" ");
	                for (String part : parts) {
	                    if (part.startsWith("/"))
	                        if (!part.toLowerCase(Locale.US).contains("vold"))
	                            out.add(part);
	                }
	            }
	        }
	    }
	    return out;
	}
	

	public static String getExternalStoragePath() {
		String state = android.os.Environment.getExternalStorageState();

		if (android.os.Environment.MEDIA_MOUNTED.equals(state)) {
			if (android.os.Environment.getExternalStorageDirectory().canWrite()) {
				return android.os.Environment.getExternalStorageDirectory()
						.getPath();
			}
		}

		return "";
	}


	public static List<FileProperty> GetFolderList(String sFolderPath,int iFrom) {
		File file = new File(sFolderPath);

		File[] files = file.listFiles();
		if (files.length == 0 ) {
			return null;
		}

		int iFileCount = files.length;
		if (iFrom >= iFileCount){
			return null;
		}
		
		int iCount = 0;		
		List<FileProperty> list = new ArrayList<FileProperty>();

		for (int i = iFrom; i < iFileCount; ++i) {
			FileProperty f = new FileProperty();
			f.m_sFileName = URLEncoder.encode(EAUtil.CHECK_STRING(
					files[i].getName(), "noname"));
			f.m_iFileSize = files[i].length();
			if (files[i].isDirectory()) {
				f.m_iFileType = EA_FILE_TYPE_DIR;
			} else {
				f.m_iFileType = GetFileType(f.m_sFileName);
			}

			list.add(f);
			if (iCount > MAX_FILE_COUNT){
				break;
			}
		}

		return list;
	}

	static public boolean RemoveFile(String sFolderPath) {
		File file = new File(sFolderPath);
		if (!file.exists()) {
			return false;
		}

		if (file.isFile()) {
			return file.delete();
		}

		File[] files = file.listFiles();
		if (files.length == 0) {
			return file.delete();
		}

		int iFileCount = files.length;
		for (int i = 0; i < iFileCount; ++i) {
			if (files[i].isDirectory()) {
				return RemoveFile(files[i].getAbsolutePath());
			} else {
				if (!files[i].delete()) {
					return false;
				}
			}
		}

		return file.delete();
	}

	static public int GetFileType(String sFilePath) {
		String sType = MimeTypes.GetMimeTypes(sFilePath);
		if (sType.contains("image")) {
			return EA_FILE_TYPE_IMAGE;
		} else if (sType.contains("audio")) {
			return EA_FILE_TYPE_AUDIO;
		} else if (sType.contains("video")) {
			return EA_FILE_TYPE_VIDEO;
		} else if (sType.contains("text") || sType.contains("msword")) {
			return EA_FILE_TYPE_TXT;
		} else if (sType.contains("zip") || sType.contains("rar") || sType.contains("tar")){
			return EA_FILE_TYPE_ZIP;
		}else if (sType.contains("octet-stream") ){
			return EA_FILE_TYPE_APP;
		}else if (sType.contains("package-archive") ){
			return EA_FILE_TYPE_APK;
		}
		
		

		return EA_FILE_TYPE_UNKNOW;
	}

	static public boolean Rename(String sFileName, String sNewFileName) {
		try {
			File resFile = new File(sFileName);
			if (!resFile.exists()) {
				return false;
			}

			return resFile.renameTo(new File(sNewFileName));
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return false;
	}

	static public boolean CreateFolder(String sFileName) {
		try {
			File resFile = new File(sFileName);
			if (!resFile.exists()) {
				return resFile.mkdir();
			}

			return true;
		} catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return false;
	}

	static public FileInputStream GetFile(String sFileName) {
		try {
			File resFile = new File(sFileName);
			if (!resFile.exists()) {
				return null;
			}

			return new FileInputStream(resFile);
		} catch (FileNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return null;
	}
}
