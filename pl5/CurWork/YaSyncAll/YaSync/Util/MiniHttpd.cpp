// WebSvr.cpp : implementation file
//

#include "stdafx.h"
#include "MiniHttpd.h"




// CMiniHttpd

CMiniHttpd::CMiniHttpd()
{
}

CMiniHttpd::~CMiniHttpd()
{
}


// CMiniHttpd member functions

void CMiniHttpd::OnAccept(int nErrorCode)
{
	CClientSock *pSock = new CClientSock();
	pSock->m_hSocket = INVALID_SOCKET;

	if (FALSE == Accept(*pSock))
	{
		CAsyncSocket::OnAccept(nErrorCode);
		return;
	}

	CAsyncSocket::OnAccept(nErrorCode);
}
