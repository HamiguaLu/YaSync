#ifndef	__PE_ADB_HELPER_HEADER__
#define __PE_ADB_HELPER_HEADER__




#define BUFSIZE 4096

class CAdbHelper
{
public:
	CAdbHelper();

	static DWORD WINAPI AdbScannerT(LPVOID  lparam);
	int StartAdbProxy(int iPort);
	int StartApp();
	int GetDeviceState();
	int InstallApp2Device();

	static HANDLE m_hExitEvt;
private:
	
	HANDLE m_hChildStdRd;
	HANDLE m_hChildStdWr;

	TCHAR m_tszCmd[1024];
	char m_szAdbOut[BUFSIZE];
	
	void CreateChildProcess(void); 
	void ReadFromPipe(void); 
	
	//int m_iPort = 0;

	int startadb();

};

#endif

