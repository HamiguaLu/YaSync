#ifndef __POCKET_EXPORT_DLL_HELPER__
#define __POCKET_EXPORT_DLL_HELPER__


#include "PEProtocol.h"

#define MAX_ENTRYID_LEN		1024


void WriteLog(TCHAR *szLogInfo);
int PEIsRegistered(void);
int IsInstanceExist(void);
int StartYaSync(void);
int SendCmd2YaSync(DWORD dwCmd,void *cmdData,int iDataLen);

#endif
