package mobitnt.media;

import java.net.URLEncoder;

import mobitnt.util.EAUtil;


public class EAImage {
	private int id;
	private String title;
	private String displayName;
	private String mimeType;
	private String path;
	private long size;
	private String galleryName;

	public EAImage() {
		super();
	}

	/**
	 * * @param id * @param title * @param displayName * @param mimeType * @param
	 * path * @param size
	 */
	public EAImage(int id, String sTitle, String sDisplayName, String sGalleryName,String sPath) {
		sTitle = EAUtil.CHECK_STRING(sTitle, "no-name");
		sPath = EAUtil.CHECK_STRING(sPath, "unkown");
		sDisplayName = EAUtil.CHECK_STRING(sDisplayName, "unkown");
		sGalleryName = EAUtil.CHECK_STRING(sGalleryName, "unkown");
		
		this.id = id;
		this.title = URLEncoder.encode(sTitle);
		this.displayName = URLEncoder.encode(sDisplayName);
		this.galleryName = URLEncoder.encode(sGalleryName);
		this.path = URLEncoder.encode(sPath);
		
/*		this.mimeType = mimeType;
		this.size = size;*/
	}

	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public String getTitle() {
		return title;
	}

	public void setTitle(String title) {
		this.title = title;
	}

	public String getDisplayName() {
		return displayName;
	}

	public void setDisplayName(String displayName) {
		this.displayName = displayName;
	}

	public String getMimeType() {
		return mimeType;
	}

	public void setMimeType(String mimeType) {
		this.mimeType = mimeType;
	}

	public String getGalleryName(){
		return galleryName;
	}
	
	public String getPath() {
		return path;
	}

	public void setPath(String path) {
		this.path = path;
	}

	public long getSize() {
		return size;
	}

	public void setSize(long size) {
		this.size = size;
	}
}