/*
EA js 通用函数
lugang at 2010-08-15
*/

var ValSeparator = "&ensp;";


String.prototype.len=function(){                    
    return this.replace(/[^\x00-\xff]/g,"rr").length;             
};

String.prototype.sub = function(n){       
    var r = /[^\x00-\xff]/g;
	if (this.length <= n){
		return this.toString();
	}
	
    if(this.replace(r, "mm").length <= n)    
        return this.toString();      
  
    var m = n;//Math.floor(n/2);       
    for(var i=m; i<this.length; i++) {       
        if(this.substr(0, i).replace(r, "mm").length>=n) {    
            return this.substr(0, i) + "...";    
        }   
    }    
    return this.toString();      
};

String.prototype.unescapeHtmlChar=function(){
	var tmp = this.toString();
	tmp = tmp.replace("&lt;","<");
	tmp = tmp.replace("&gt;",">");
	/*tmp = tmp.replace("&lt;","<");
	tmp = tmp.replace("&lt;","<");
	tmp = tmp.replace("&lt;","<");*/
	return tmp;
}

Array.prototype.del = function(n) {
	if (n < 0) {
		return this;// 如果n<0，则不进行任何操作。
	}
	
	this.splice(n, 1);
};



var MAX_ITEM_PER_REQUEST = 25;


function ShowAlert(info) {
	$("#AlertContent").html(info);
	$.colorbox({
		inline : true,
		href : '#AlertContent'
	});
}


function OnAjaxReqSuccess(xml) {
	
}

function OnAjaxReqFailed(XMLHttpRequest, textStatus, errorThrown) {
	
	var sErr = "http-status:" + XMLHttpRequest.status;
	sErr += " http-state:" + XMLHttpRequest.readyState;
	sErr += " Errortext:" + textStatus;
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
