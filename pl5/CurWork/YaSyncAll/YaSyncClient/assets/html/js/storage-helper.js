var g_iFileTotalPage = 0;
var g_iFileStartItem = 0;
var FILE_ITEM_PER_PAGE = 10;

var g_sCurrentFolder = "/";
var g_sRootFolder = "/";

var FILE_TYPE_TEXT = 1;
var FILE_TYPE_AUDIO = 2;
var FILE_TYPE_IMAGE = 3;
var FILE_TYPE_VIDEO = 4;
var FILE_TYPE_FOLDER = 5;
var FILE_TYPE_ZIP = 6;
var EA_FILE_TYPE_APP = 7;
var EA_FILE_TYPE_APK = 8;
var MAX_FILE_NAME_LEN = 10;

var EA_RET_END_OF_FILE = 7;

var sLUpload = "Upload";
var sLRefresh = "Refresh";
var sLDeleteFile = "Delete file";
var sLNewFolder = "Create folder";
var sLReNameFile = "ReName";
var sLFileName = "Name";
var sLNewFileName = "New Name";
var sLSelectFile = "Please select one file";
var sLConfirmeDelete = "Are you sure you want to delete:";
var sLRootFolder = "/";

function checkPath(sPath){
	sPath = sPath.replace("////","/");
	sPath = sPath.replace("///","/");
	sPath = sPath.replace("//","/");
	return sPath;
}

function OnQueryFailed(){
	
}

function OnSdCardChanged(){
	g_sRootFolder = $("#PathListSel").val();
	g_sRootFolder += "/";
	g_sRootFolder = checkPath(g_sRootFolder);
	$("#FileList").html("");
	g_iFileStartItem = 0;
	GetFileList(g_sRootFolder);
}

function SDCardListXmlParser(xml) {
	//$("#PathListSel").html("");
	$(xml).find("path").each(
			function(i) {
				var sPath = decodeURIComponent($(this).find("value").text());
				var sDefault = $(this).find("def").text();
				var sel = "";
				if (sDefault == 1){
					sel = "selected";
				}
				var sHtml = "<option value='" + sPath + "' " + sel + ">" + sPath + "</option>";
				$('#PathListSel').append(sHtml);
			});
		
	//alert($('#PathListSel option').length);
	if ($('#PathListSel option').length < 2){
		$('#sdcardList').hide();
	}else{
		$('#sdcardList').show();
	}
	
	OnSdCardChanged();
	
}

function GetSDCardList(){
	var sReq = "FolderList.xml?action=getsdcardlist";
	StartAjaxRequest(sReq,"xml",SDCardListXmlParser,OnQueryFailed);
}

function StorageXmlParser(xml) {
	FolderXmlParser(xml);
	GenCurrentFolderLink();
}

function GetFileList(sFolderName) {

	var sReq = "FolderList.xml?action=getfolderlist&file=" + sFolderName;
	sReq += "&" + "from=" + g_iFileStartItem;
	//sReq += "&to=" + (g_iFileStartItem + FILE_ITEM_PER_PAGE);
	StartAjaxRequest(sReq,"xml",StorageXmlParser,OnQueryFailed);
}

function GetRootFolder(){
	var sReq = "FolderList.xml?action=getexternalrootfolder";
	StartAjaxRequest(sReq,"xml",StorageXmlParser,OnQueryFailed);
}

function GetRealPath(path){
	return checkPath(g_sRootFolder + "/" + path);
}

function OnClickEnterFolder(sFolderName) {
	g_iFileStartItem = 0;
	var sPath = g_sRootFolder + "/" + g_sCurrentFolder + "//" + sFolderName;
	GetFileList(checkPath(sPath));

}

function OnClickEnterFolderEx(sParentFolder) {
	g_iFileStartItem = 0;
	var sPath = g_sRootFolder + "/" + sParentFolder;
	GetFileList(checkPath(sPath));
}

/*
 * <li><a href="index.html">my document >></a></li> <li class="current-tab"><a
 * href="style-demo.html">picture >></a></li> <li><a
 * href="style-demo.html">photos</a></li>
 */

function GenCurrentFolderLink() {
	var folders = g_sCurrentFolder.split("/");
	
	var sLinks = "";
	var sFolderPath = "/";
	for ( var i = 0; i < folders.length; ++i) {
		var sFolderName = folders[i];
		if (sFolderName.length < 1) {
			if (i == 0) {
				sFolderName = "/";
			} else {
				continue;
			}

		}
		sFolderPath += folders[i];
		
		var sClass = 'class="pathItem  button-transition';
		if (i == folders.length - 1){
			sClass += ' path-current';
		}
		sClass += '"';

		sLinks += '<a href="#" ' +  sClass + ' onclick=OnClickEnterFolderEx(\"' + sFolderPath
				+ '");>' + sFolderName + '</a>';
		
		sFolderPath += "/";
	}

	$("#FolderPath").html(sLinks);
}

function bytesToSize(bytes) {
    var sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
    if (bytes == 0) return 'n/a';
    var i = parseInt(Math.floor(Math.log(bytes) / Math.log(1024)));
    return Math.round(bytes / Math.pow(1024, i), 2) + ' ' + sizes[i];
}


function FolderGenLink(sFileName, iSize, iType) {
	var filesize = bytesToSize(iSize);
	var sSubFileName = decodeURIComponent(sFileName);
	sSubFileName = sSubFileName.sub(MAX_FILE_NAME_LEN);
	var sTitle = ' title = "' + decodeURIComponent(sFileName) + ' ' + filesize + '"' ;
	var sImageClass = '><img src="img/other-file.png"';
	var sPageLink = '<a href="?action=getfile&file=' +  GetRealPath(g_sCurrentFolder + "/" + sFileName) +'" target="_blank"';
	if (iType == FILE_TYPE_FOLDER) {
		sImageClass = '><img src="img/folder.png"';
		sTitle = ' title = "' + decodeURIComponent(sFileName) + '"' ;
		sPageLink = '<a href="#" onclick="OnClickEnterFolder(\'' + sFileName + '\');"';
	} else if (iType == FILE_TYPE_IMAGE) {
		sImageClass = '><img src="img/photo.png"';
	} else if (iType == FILE_TYPE_AUDIO) {
		sImageClass = '><img src="img/music.png"';
	} else if (iType == FILE_TYPE_VIDEO) {
		sImageClass = '><img src="img/video.png"';
	} else if (iType == FILE_TYPE_TEXT) {
		sImageClass = '><img src="img/doc.png"';
	} else if (iType == FILE_TYPE_ZIP) {
		sImageClass = '><img src="img/zip.png"';
	} else if (iType == EA_FILE_TYPE_APP) {
		sImageClass = '><img src="img/app.png"';
	} else if (iType == EA_FILE_TYPE_APK) {
		sImageClass = '><img src="img/apk.png"';
	}
	
	sImageClass += sTitle + '/>';
	sPageLink += sImageClass;
	
	var sLink = '<dl class="fileItem">';
	sLink += '<dt>' + sPageLink + '</a>' + '</dt>';
	sLink += '<dd>' + sSubFileName + '</dd>';
	sLink += '<dd><input type="checkbox" name="FileCB" value="' + decodeURIComponent(sFileName) + '"/></dd>'; 
	
	sLink += "</dl>";
	
	//LOG4MB(sLink);

	return sLink;
}

var firstTime = 1;
function FolderXmlParser(xml) {
	var retVal = $(xml).find("EARetCode");
	if (retVal.text().length >= 1) {
		var retcode = retVal.text();
		if (parseInt(retcode) == EA_RET_END_OF_FILE){
			return;
		}
		
		ShowAlert(retcode);
		return;
	}
	
	retVal = $(xml).find("RootPath");
	if (retVal.text().length > 1) {
		g_sRootFolder = retVal.text();
		g_sRootFolder += "/";
		
		$("#FileList").html("");
		GetFileList(g_sCurrentFolder);
		
		return;
	}
	
	retVal = $(xml).find("Refresh");
	if (retVal.text().length > 1) {
		OnRefreshContent();
		return;
	} 

	var TotalCount = $(xml).find("TotalCount").text();

	var cFolder = $(xml).find("CurrentFolder");
	if (cFolder.text() != g_sCurrentFolder || g_iFileStartItem == 0) {
		// current folder has been changed already,so start from first page
		g_iFileStartItem = 0;
		g_sCurrentFolder = cFolder.text();
		g_sCurrentFolder = g_sCurrentFolder.replace(g_sRootFolder,"//");
		$("#FileList").html("");
	}
	
	if (parseInt(TotalCount) == 0){
		$("#FileList").html("");
		return;
	}
	
	$(xml).find("Folder").each(
			function(i) {
				var fLink = FolderGenLink($(this).find("Name").text(), $(this)
						.find("Size").text(), $(this).find("Type").text());
				$("#FileList").append(fLink);
				
				g_iFileStartItem += 1;
			});
	
	if (g_iFileStartItem < parseInt(TotalCount)){
		GetFileList(g_sCurrentFolder);
		return;
	}
}


function OnSumbitFile(id, filename) {
	uploader.setParams({
		"file" : g_sCurrentFolder
	});
}

function OnDeleteFile(){
	var check_obj = $("input:checked[name='FileCB']");
	var sFileList = "";

	var sFiles = "";
	for ( var i = 0; i < check_obj.length; i++) {
		if (i > 0){
			sFileList += ",";
			sFiles += ",";
		}
		
		sFileList += checkPath(GetCurrentFolder() + "//" + check_obj.get(i).value);
		sFiles += check_obj.get(i).value;
	}
	
	if(confirm(sLConfirmeDelete + '<' + sFiles +'>?')){
		var sReq = "FolderList.xml?action=removefile&filelist=" + encodeURIComponent(sFileList);
		StartAjaxRequest(sReq,"xml",StorageXmlParser,OnQueryFailed);
	}
}

function createFileNow(){
	var name = $("#CREATE-FOLDER-NAME").val();
	var sPath = checkPath(GetCurrentFolder() + "/" + name);
	var sReq = "FolderList.xml?action=createfile&file=" + sPath;
	StartAjaxRequest(sReq,"xml",StorageXmlParser,OnQueryFailed);
	
	$.colorbox.close();
}

function OnCreateFile(){
	$.colorbox({
		inline : true,
		href : '#CREATE-FILE-DIV'
	});
}

function ReNameNow(){
	var sFileName = $("#RENAME-FOLDER-NAME").val();
	var sNewFileName = $("#RENAME-FOLDER-NAME-NEW").val();
	
	if (sFileName == sNewFileName){
		return;
	}
	
	var sPath = g_sRootFolder + "/" + g_sCurrentFolder + "/" + sFileName;
	sPath = checkPath(sPath);
	var sReq = "FolderList.xml?action=renamefile&file=" + sPath;
	sReq += '&newpath=' + checkPath(GetCurrentFolder() + "//" + sNewFileName);
	StartAjaxRequest(sReq,"xml",StorageXmlParser,OnQueryFailed);
	
	$.colorbox.close();
}


function OnRenameFile(){
	var check_obj = $("input:checked[name='FileCB']");
	
	if( check_obj.length > 0) {
		var sFileName = check_obj.get(0).value;
		$("#RENAME-FOLDER-NAME").val(sFileName);
		$("#RENAME-FOLDER-NAME-NEW").val(sFileName);
	}
	else {
		ShowAlert(sLSelectFile);
		return;
	}
	
	$.colorbox({
		inline : true,
		href : '#RENAME-FILE-DIV'
	});
}

function OnRefreshContent(){
	g_iFileStartItem = 0;
	
	var sPath = g_sRootFolder + "/" + g_sCurrentFolder;
	GetFileList(checkPath(sPath));
		
	$("#FileList").html("");
	
}

function GetCurrentFolder(){
	return g_sRootFolder + "/" + g_sCurrentFolder;
}

function ShowUploadDiv(){
	$.colorbox({
		inline : true,
		href : '#UPLOAD-FILE-DIV'
	});
}


$(document).ready(function() {
	g_iFileTotalPage = 0;
	g_iFileStartItem = 0;

	g_sCurrentFolder = "/";
	
	GetSDCardList();
		
	$('#sdcardList').hide();
	
	$("#FolderPath").html("");
	$("#FileList").html("");
	
	$('#file_upload').uploadify({
				'swf'      : '/uploadify.swf',
				'debug'    : false,
				'uploader' : '/uploadify.xml',
				 'onUploadSuccess' : function(file, data1, response) {
					//alert(response);
					OnRefreshContent();
					alert('The file ' + file.name + ' was successfully uploaded:' + data1);
					
				},
				'onUploadStart' : function(file) {
					//alert(file.size);
					var sPath = g_sRootFolder + "/" + g_sCurrentFolder + "/" + file.name;
					sPath = checkPath(sPath);
					$('#file_upload').uploadify('settings','formData',{'currentfolder':sPath, 'filesize' : file.size, 'action' : 'uploadfile' });
				}
			});
			
			
	$("#PathListSel").change(function() { OnSdCardChanged();});		
});
