#pragma once

// CClientSock command target

class CClientSock : public CSocket
{
public:
	CClientSock();
	virtual ~CClientSock();
	virtual void OnReceive(int nErrorCode);
private:
	void ServeRequest();
	int OnAjaxRequest(char * szReqest,char *szParameter,char *szContentType);
	void SendResponseHeader(int iLen,char *sType);
	void SendErrorResponse(int iCode,char *szData);
	char *GetParameterVal(char *szParameterList,char *szPName,char *sDefVal);

	void SendRetXml(char *szmsg);
	void SendRetXml(char *szMsg,char *szTag);
	void SendRawStr(char *szMsg);
	
};


