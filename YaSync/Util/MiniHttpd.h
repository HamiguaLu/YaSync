#pragma once

// CWebSvr command target
#include "ClientSock.h"
class CMiniHttpd : public CAsyncSocket
{
public:
	CMiniHttpd();
	virtual ~CMiniHttpd();
	int m_nPort;
	CClientSock m_client;
private:
	virtual void OnAccept(int nErrorCode);
	

};


