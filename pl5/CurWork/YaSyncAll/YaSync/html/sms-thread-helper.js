
/*
EA js 通用函数
lugang at 2010-08-15
*/


function OnClose(){
	var sReq = "ajaxclose.xml?action=close";
	StartAjaxRequest(sReq,"xml",null,null);
}

function OnAjaxReqSuccess(xml) {
	
}

function OnAjaxReqFailed(XMLHttpRequest, textStatus, errorThrown) {
	
	var sErr = "http-status:" + XMLHttpRequest.status;
	sErr += " http-state:" + XMLHttpRequest.readyState;
	sErr += " Errortext:" + textStatus;
	alert(sErr);
}


// 扩展AJAX请求函数, 传递自定义回调函数
function StartAjaxRequest(xmlUrl, acceptDataType, OnReqSuccess, OnReqFailed)
{
	$.ajaxSetup({
		cache : false
	});
	
	if (acceptDataType == null){
		acceptDataType = "xml";
	}
	
	xmlUrl = xmlUrl + "&" + "mobitnttimestamp=" + Math.random();
	
	if (OnReqSuccess == null){
		OnReqSuccess = OnAjaxReqSuccess;
	}
	
	if (OnReqFailed == null){
		OnReqFailed = OnAjaxReqFailed;
	}

	$.ajax({
		url : xmlUrl,
		type : "get",
		dataType:acceptDataType,
		success : function(msg) {
			OnReqSuccess(msg);
		},
		error : function(XMLHttpRequest, textStatus, errorThrown) {
			OnReqFailed(XMLHttpRequest, textStatus, errorThrown);
		}
	});
}



function OnTResponse(html){
	$("#chatList").html(html);
}

function OnTQueryFailed(){
	$("#chatList").html("Failed to get sms list");
}

function OnDeleteSMS(threadid,msgid){
	var sUrl = "SmsList.xml?action=deletesms&threadid=" + threadid + "&id=" + msgid;
	StartAjaxRequest(sReq, "html",OnTResponse,OnTQueryFailed);
}

$(document).ready(function() {
	var sHtml = "<div class='loading'><img src='smsloading.gif'/>Please wait</div>";
	$("#chatList").html(sHtml);
	
	var sReq = "ajaxsmsthread.xml?action=getchatlist";
	StartAjaxRequest(sReq, "html",OnTResponse,OnTQueryFailed);
	
	$("#CloseBtn").click(function(){
		var sReq = "ajaxclose.xml?action=close";
		StartAjaxRequest(sReq,"xml",null,null);
	})
});

