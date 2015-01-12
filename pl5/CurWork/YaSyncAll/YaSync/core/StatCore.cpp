#include "stdafx.h"
#include "PECore.h"

#include "StatDlg.h"



int GenChartData(TCHAR *szName,TCHAR *szPhone)
{
	SMS_STAT *pSmsStat = (SMS_STAT*)calloc(1,sizeof(SMS_STAT));
	CALL_LOG_STAT *pCallStat = (CALL_LOG_STAT*)calloc(1,sizeof(CALL_LOG_STAT));

	_tcscpy(pSmsStat->szName,szName);
	_tcscpy(pSmsStat->szPhone,szPhone);

	_tcscpy(pCallStat->szName,szName);
	_tcscpy(pCallStat->szPhone,szPhone);
	

	int iRet = PECore::GenStatData(pSmsStat,pCallStat);
	if (iRet != PE_RET_OK)
	{
		free(pSmsStat);
		free(pCallStat);
		return iRet;
	}


	pCallStat->ulInTime = pCallStat->ulInTime/60;
	pCallStat->ulOutTime = pCallStat->ulOutTime/60;

	CStatDlg *pDlg = new CStatDlg();
	pDlg->SetStatInfo(pSmsStat,pCallStat);
	pDlg->DoModal();

	delete pDlg;


	free(pSmsStat);
	free(pCallStat);
	
	return PE_RET_OK;

}






