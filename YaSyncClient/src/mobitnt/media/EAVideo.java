package mobitnt.media;

import java.net.URLEncoder;

import mobitnt.util.EAUtil;

public class EAVideo {
	private int id;
	private String title;
	private String album;
	private String artist;
	private String displayName;
	private String path;
	
/*	private String mimeType;
	private long size;
	private long duration;*/

	/**     

*      */
	public EAVideo() {

	}

	/**
	 * * @param id * @param title * @param album * @param
	 * 
	 * artist * @param displayName * @param mimeType * @param data * @param size
	 * * @param duration
	 */
	public EAVideo(int id, String sTitle, String sAlbum, String sArtist,
			String sDisplayName, String sPath) {
		sTitle = EAUtil.CHECK_STRING(sTitle, "no-name");
		sAlbum = EAUtil.CHECK_STRING(sAlbum, "unkown");
		sArtist = EAUtil.CHECK_STRING(sArtist, "unkown");
		sPath = EAUtil.CHECK_STRING(sPath, "unkown");
		sDisplayName = EAUtil.CHECK_STRING(sDisplayName, "unkown");
		
		this.id = id;
		this.title =  URLEncoder.encode(sTitle);
		this.album =  URLEncoder.encode(sAlbum);

		this.artist =  URLEncoder.encode(sArtist);
		this.displayName =  URLEncoder.encode(sDisplayName);
		this.path =  URLEncoder.encode(sPath);

		//this.size = size;
		//this.duration = duration;
	}

	public int getId() {
		return id;
	}

	public void setId(int

	id) {
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

	public String getDisplayName() {
		return displayName;
	}

	public void setDisplayName

	(String displayName) {
		this.displayName = displayName;
	}


	public String getPath() {
		return path;

	}

	public void setPath(String path) {
		this.path = path;
	}

}