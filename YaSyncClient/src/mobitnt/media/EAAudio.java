package mobitnt.media;

import java.net.URLEncoder;

import mobitnt.util.EAUtil;

public class EAAudio {
	private int id;
	private String title;
	private String album;
	private String artist;
	private String path;
	private String displayName;
	private String mimeType;
	private long duration;
	private long size;

	/** * */
	public EAAudio() {
		
	}

	/**
	 * * @param id * @param title * @param album * @param artist * @param path * @param
	 * displayName * @param mimeType * @param duration * @param size
	 */
	public EAAudio(int id, String sTitle, String sAlbum, String sArtist,
			String sPath, String sDisplayName) {
		
		
		sTitle = EAUtil.CHECK_STRING(sTitle, "no-name");
		sAlbum = EAUtil.CHECK_STRING(sAlbum, "unkown");
		sArtist = EAUtil.CHECK_STRING(sArtist, "unkown");
		sPath = EAUtil.CHECK_STRING(sPath, "unkown");
		sDisplayName = EAUtil.CHECK_STRING(sDisplayName, "unkown");
		
		this.id = id;
		this.title = URLEncoder.encode(sTitle);
		this.album = URLEncoder.encode(sAlbum);
		this.artist = URLEncoder.encode(sArtist);
		this.path = URLEncoder.encode(sPath);
		this.displayName = URLEncoder.encode(sDisplayName);
	/*	this.mimeType = sMimeType;
		this.duration = duration;
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

	public String getAlbum() {
		return album;
	}

	public void setAlbum(String album) {
		this.album = album;
	}

	public String getArtist() {
		return artist;
	}

	public void setArtist(String artist) {
		this.artist = artist;
	}

	public String getPath() {
		return path;
	}

	public void setPath(String path) {
		this.path = path;
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

	public long getDuration() {
		return duration;
	}

	public void setDuration(long duration) {
		this.duration = duration;
	}

	public long getSize() {
		return size;
	}

	public void setSize(long size) {
		this.size = size;
	}
}
