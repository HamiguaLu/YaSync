
var MAX_APP_NAME_LEN = 10;

function GetID(name){
	var s = name.replace(/\./g,"");
	return s;
}

function OnQueryFailed(){
	
}

function GetAppList(action) {
	var sReq = "AppList.xml?action=getapplist";
	StartAjaxRequest(sReq,"xml",AppXmlParser,OnQueryFailed);
}

function OnClickRemvoeApp(sAppName) {
	if(!confirm("Are you sure to delete" + '<' + sAppName +'>? ')){
		return;
	}
	
	var sReq = "AppList.xml?action=removeapp&appname=" + sAppName;
	StartAjaxRequest(sReq,"xml");
	
	ShowAlert("Please take a look on your device's screen");
	var sID = "#" + GetID(sAppName);
	$(sID).remove();
}

function AppGenLink(sName, sPackageName, sVersionName) {
	var sSubName = decodeURIComponent(sName);
	sSubName = sSubName.sub(MAX_APP_NAME_LEN);
	var sLink = '<dl class="fileItem" id="' + GetID(sPackageName) + '">';

	sLink += "<dt><img width='48' height='48' src=\"?action=getappicon&appname="
			+ sName + "\" title=\"" + decodeURIComponent(sPackageName) + " V" + decodeURIComponent(sVersionName) + "\"></dt>";
	
	sLink += "<dd>";
	sLink += sSubName;
	sLink += "</dd>";
	
	sLink += '<a href="#" onclick="OnClickRemvoeApp(\'' + sPackageName + '\');\"><img src="img/delete.png"/></a>';

	sLink += '</dl>';
	return sLink;
}

function AppXmlParser(xml) {
	var bHaveData = 0;
	
	$(xml).find("App").each(
			function(i) {
				var fLink = AppGenLink($(this).find("Name").text(), $(this)
						.find("PackageName").text(), $(this)
						.find("VersionName").text());
				$("#app-list").append(fLink);
				bHaveData = 1;
			});

}

function OnInstallApp(){
	$.colorbox({
		inline : true,
		href : '#UPLOAD-FILE-DIV'
	});
}

function OnRefreshApp(){
	GetAppList();
	$("#app-list").html("");
}

function OnAppInit() {
	GetAppList();
	$("#app-list").html("");
		$('#file_upload').uploadify({
				'swf'      : '/uploadify.swf',
				'debug'    : false,
				'uploader' : '/uploadify.xml',
				'onUploadStart' : function(file) {
					$('#file_upload').uploadify('settings','formData',{'action' : 'installapk'});
					//alert(file.name);
				},
				'onUploadError' : function(file, errorCode, errorMsg, errorString) {
					alert('The apk ' + file.name + ' could not be uploaded: ' + errorString);
				},
				'onUploadSuccess' : function(file, data, response) {
					alert('Please take a look on your device screen!');
				} 
			});
			
	//ShowAlert("Please take a look on your device's screen");		
}

$(document).ready(function() {
	OnAppInit();
});
