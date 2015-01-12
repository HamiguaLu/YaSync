// HomeUI.cpp : implementation file
//

#include "stdafx.h"

#include "YaSync.h"
#include "HomeUI.h"

#include "PECore.h"
// CHomeUI dialog

IMPLEMENT_DYNAMIC(CHomeUI, CDialog)

HWND g_hHomeUIWnd = 0;
extern int g_iCanSync;


CHomeUI::CHomeUI(CWnd* pParent /*=NULL*/)
: CDialog(CHomeUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CHomeUI::~CHomeUI()
{
}

void CHomeUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP_ADDR, m_DevIP);
	DDX_Control(pDX, IDC_PROGRESS_BATTERY, m_batteryLevel);
	DDX_Control(pDX, IDC_COMBO_CONNECTED_PHONES, m_ConnPhones);
	DDX_Control(pDX, IDC_PROGRESS_SDCARD, m_progSdCard);
}


BEGIN_MESSAGE_MAP(CHomeUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CHomeUI::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_SWITCH_2_PHONE_BTN, &CHomeUI::OnBnClickedSwitch2PhoneBtn)
	ON_CBN_SELCHANGE(IDC_COMBO_CONNECTED_PHONES, &CHomeUI::OnCbnSelchangeComboConnectedPhones)
	ON_BN_CLICKED(IDC_BTN_NEW_SMS, &CHomeUI::OnBnClickedBtnNewSms)
	ON_BN_CLICKED(IDC_BTN_PHOTO, &CHomeUI::OnBnClickedBtnPhoto)
	ON_BN_CLICKED(IDC_BTN_APP_MANAGER, &CHomeUI::OnBnClickedBtnAppManager)
	ON_BN_CLICKED(IDC_BTN_FILE_MANAGER, &CHomeUI::OnBnClickedBtnFile)
	ON_BN_CLICKED(IDC_BTN_RESTORE, &CHomeUI::OnBnClickedBtnRestore)
	ON_BN_CLICKED(IDC_BTN_QUICK_START, &CHomeUI::OnBnClickedBtnQuickStart)
END_MESSAGE_MAP()


// CHomeUI message handlers

BOOL CHomeUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnConnect.SubclassDlgItem(IDC_BTN_CONNECT, this);
	m_btnConnect.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnConnect.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnConnect.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnConnect.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnConnect.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnConnect.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	m_btnSwitch.SubclassDlgItem(IDC_SWITCH_2_PHONE_BTN, this);
	m_btnSwitch.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnSwitch.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnSwitch.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnSwitch.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnSwitch.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnSwitch.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);
	m_btnSwitch.EnableWindow(0);

	m_btnQuickStart.SubclassDlgItem(IDC_BTN_QUICK_START, this);
	m_btnQuickStart.SetAlign(CButtonST::ST_ALIGN_VERT);
	m_btnQuickStart.SetIcon(IDI_QUICK_START);

	m_btnNewSMS.SubclassDlgItem(IDC_BTN_NEW_SMS, this);
	m_btnRestore.SubclassDlgItem(IDC_BTN_RESTORE, this);
	m_btnPhoto.SubclassDlgItem(IDC_BTN_PHOTO, this);
	m_btnApp.SubclassDlgItem(IDC_BTN_APP_MANAGER, this);
	m_btnFile.SubclassDlgItem(IDC_BTN_FILE_MANAGER, this);

	m_btnNewSMS.SetAlign(CButtonST::ST_ALIGN_VERT);
	m_btnRestore.SetAlign(CButtonST::ST_ALIGN_VERT);
	m_btnPhoto.SetAlign(CButtonST::ST_ALIGN_VERT);
	m_btnApp.SetAlign(CButtonST::ST_ALIGN_VERT);
	m_btnFile.SetAlign(CButtonST::ST_ALIGN_VERT);

	m_btnNewSMS.SetIcon(IDI_SMS_EDITOR);
	m_btnRestore.SetIcon(IDI_RESTORE_DLG);
	m_btnPhoto.SetIcon(IDI_IMAGE_VIEWER);
	m_btnApp.SetIcon(IDI_APK_INSTALLER);
	m_btnFile.SetIcon(IDI_FILE_TRANSFER);

	m_ConnPhones.ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_CONN_PHONES)->ShowWindow(SW_HIDE);
	m_btnSwitch.ShowWindow(SW_HIDE);

	m_batteryLevel.SetWindowText(_T("Battery level"));
	m_batteryLevel.SetRange(0,100);
	m_batteryLevel.SetPos(0);
	m_batteryLevel.SetBarColor(SUB_BTN_COLOR);
	m_batteryLevel.SetShowPercent(FALSE);

	m_progSdCard.SetWindowText(_T("SD Card:"));
	m_progSdCard.SetRange(0,100);
	m_progSdCard.SetPos(0);
	m_progSdCard.SetBarColor(SUB_BTN3_COLOR);
	m_progSdCard.SetShowPercent(FALSE);

	g_hHomeUIWnd = m_hWnd;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

CString FormatBytes(__int64 bytes)
{
	TCHAR *szSuffix[] = { _T("B"), _T("KB"), _T("MB"), _T("GB"), _T("TB") };
	int i = 0;
	double dblSByte = (double)bytes;
	if (bytes > 1024)
	{
		for (i = 0; (bytes / 1024) > 0; i++, bytes /= 1024)
		{
			dblSByte = bytes / 1024.0;
		}
	}

	CString s;
	s.Format(_T("%d"),(int)dblSByte);
	s += szSuffix[i];

	return s;
}


void CHomeUI::OnDeviceInfo(PE_DEV_INFO *pInfo)
{
	CString sPhone;
	sPhone.Format(_T("Phone: %s, made by %s"),pInfo->szPhoneModel,pInfo->szPhoneManufacturer);
	SetDlgItemText(IDC_STATIC_PHONE_NAME,sPhone);

	CString sSDCard;
	CString sTotal = FormatBytes(pInfo->dwSDCardTotalSpace);
	float fPercent = 0;
	if (pInfo->dwSDCardTotalSpace)
	{
		fPercent = (float)((double)pInfo->dwSDCardAvailableSpace/(double)pInfo->dwSDCardTotalSpace);
		fPercent *= 100;
	}
	sSDCard.Format(_T("SDCard: total %s(%d%% free)"),sTotal.GetBuffer(),(int)fPercent);
	sTotal.ReleaseBuffer();
	m_progSdCard.SetWindowText(sSDCard);
	m_progSdCard.SetPos((int)fPercent);

	CString sPercent;
	sPercent.Format(_T("Battery:%d%% remaining"),pInfo->dwBatteryLevel);
	m_batteryLevel.SetWindowText(sPercent);
	m_batteryLevel.SetPos((int)pInfo->dwBatteryLevel);
}

void CHomeUI::OnDeviceConnectionChange(int iConnected)
{
	if (iConnected)
	{
		CString sInfo = _T("Device Connected");
		SetDlgItemText(IDC_STATIC_DEV_CONN_INFO,sInfo);
	}
	else
	{
		CString sInfo = _T("Device Not Connected");
		SetDlgItemText(IDC_STATIC_PHONE_NAME,_T("Phone:"));
		SetDlgItemText(IDC_STATIC_DEV_CONN_INFO,sInfo);
		m_batteryLevel.SetWindowText(_T("Battery level:"));
		m_batteryLevel.SetPos(0);

		m_progSdCard.SetWindowText(_T("SD Card:"));
		m_progSdCard.SetPos(0);
	}
}

void CHomeUI::OnDisableConnBtn(int iDisable)
{
	if (iDisable != 0)
	{
		m_btnConnect.EnableWindow(0);
		return;
	}

	m_btnConnect.EnableWindow(1);
}



HBRUSH CHomeUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor   ==   CTLCOLOR_STATIC)  
	{  
		pDC->SetTextColor(RGB(0,0,0));  
		pDC->SetBkColor(SUB_DLG_BK_COLOR);  
		return m_hDialogBrush;
	}
	else if (nCtlColor == CTLCOLOR_DLG)
	{
		//pDC->SetTextColor(COLOR_DLG_VC);
		return m_hDialogBrush;
	}
	return m_hDialogBrush;
}

void CHomeUI::OnBnClickedBtnConnect()
{
	if(m_DevIP.IsBlank())
	{
		PECore::PostMessage(WM_SCAN_DEVICE,0,0);
		return;
	}

	BYTE nField0 = 0, nField1 = 0, nField2 = 0, nField3 = 0;
	m_DevIP.GetAddress(nField0, nField1, nField2, nField3);

	CString sIP;
	sIP.Format(_T("%d.%d.%d.%d"), nField0, nField1, nField2, nField3);

	TCHAR *szIP = (TCHAR *)calloc(1,255);
	_tcscpy(szIP,sIP.GetBuffer());
	sIP.ReleaseBuffer();
	PECore::PostMessage(WM_CONNECT_2_DEVICE,(WPARAM)szIP,0);

	m_btnConnect.EnableWindow(0);

#ifndef _DEBUG
	PECore::PostDelayMessage(WM_DISABLE_CONN_BTN,0,0,60);
#else
	PECore::PostDelayMessage(WM_DISABLE_CONN_BTN,0,0,3);
#endif

}

extern int g_iInSync;

void CHomeUI::OnBnClickedSwitch2PhoneBtn()
{
	if (g_iInSync)
	{
		MessageBox(_T("Can not switch phone during sync"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	CString sPhoneName;
	m_ConnPhones.GetWindowText(sPhoneName);
	PECore::SendMessage(WM_SWITCH_2_DEVICE,(WPARAM)sPhoneName.GetBuffer());
	sPhoneName.ReleaseBuffer();
}

BOOL CHomeUI::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (message)
	{
	case WM_SHOW_CONN_STAT:
		{
			CString sInfo = _T("");
			switch (wParam)
			{
				case SCAN_STAT_QUERY_FROM_SRV:
				{
					 sInfo = _T("Query device IP...");
					 break;
				}
				case SCAN_STAT_CONNECT_2_CLIENT:
				{
					 sInfo = _T("Connect to device...");
					 break;
				}
				case SCAN_STAT_CONNECT_FROM_HISTORY:
				{
					 sInfo = _T("Connect to device from last IP address...");
					 break;
				}
				case SCAN_STAT_STOP:
				{
					break;
				}
			}
			SetDlgItemText(IDC_STATIC_CONN_STAT,sInfo);
			break;
		}
	case WM_DEVICE_CONNECTED:
		{
			PE_DEV_INFO *pInfo = (PE_DEV_INFO*)wParam;
			OnDeviceInfo(pInfo);
			OnDeviceConnectionChange(1);
			free(pInfo);
			break;
		}
	case WM_BATTERY_LEVEL_CHANGED:
		{
			TCHAR* szLevel = (TCHAR*)wParam;
			DWORD dwBatteryLevel = _ttoi(szLevel);

			CString sPercent;
			sPercent.Format(_T("Battery:%d%% remaining"),dwBatteryLevel);
			m_batteryLevel.SetWindowText(sPercent);
			m_batteryLevel.SetPos(dwBatteryLevel);

			break;
		}
	case WM_DEVICE_DISCONNECTED:
		{
			OnDeviceConnectionChange(0);
			break;
		}
	case WM_DISABLE_CONN_BTN:
		{
			OnDisableConnBtn((int)wParam);
			break;
		}
	case WM_DEVICE_LIST_COMING:
		{
			m_ConnPhones.ResetContent();
			PE_PHONE_SETTING *s = (PE_PHONE_SETTING*)lParam;
			list<PE_PHONE_SETTING*> *pList = (list<PE_PHONE_SETTING*> *)wParam;
			if (!s || !pList || pList->size() < 2)
			{
				m_ConnPhones.ShowWindow(SW_HIDE);
				GetDlgItem(IDC_STATIC_CONN_PHONES)->ShowWindow(SW_HIDE);
				m_btnSwitch.ShowWindow(SW_HIDE);
				return TRUE;
			}

			list<PE_PHONE_SETTING*>::iterator it;
			for (it = pList->begin(); it != pList->end(); ++it)
			{ 
				PE_PHONE_SETTING *tmp = (PE_PHONE_SETTING*)*it;
				m_ConnPhones.AddString(tmp->szPhoneName);
			}

			m_ConnPhones.SelectString(0,s->szPhoneName);

			m_ConnPhones.ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_CONN_PHONES)->ShowWindow(SW_SHOW);
			m_btnSwitch.ShowWindow(SW_SHOW);

			break;
		}
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}



void CHomeUI::OnCbnSelchangeComboConnectedPhones()
{
	m_btnSwitch.EnableWindow(1);
}



void CHomeUI::StartUI(int iUIType)
{
	TCHAR szHostURL[255];

	int iRet = GetToolURL(iUIType,szHostURL);
	if (iRet == PE_RET_DEVICE_NOT_CONNECTED)
	{
		MessageBox(_T("Device is not connected"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	if (iRet != PE_RET_OK)
	{
		return;
	}

	CUIContainer *pUIContainer = new CUIContainer();
	pUIContainer->m_width = 800;
	pUIContainer->m_height = 600;
	pUIContainer->SetURL(szHostURL);

	pUIContainer->DoModal();

	delete pUIContainer;

}



int CHomeUI::GetToolURL(int iUIType,OUT TCHAR *szHost)
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return PE_RET_DEVICE_NOT_CONNECTED;
	}

	TCHAR *szFileName = _T("");

	void *data = NULL;
	switch (iUIType)
	{
	case PE_UI_TYPE_APP:
		{
			szFileName = _T("App.html");
			break;
		}
	case PE_UI_TYPE_PHOTO:
		{
			szFileName = _T("Photo.html");
			break;
		}
	case PE_UI_TYPE_FILE:
		{
			szFileName = _T("File.html");
			break;
		}
	default:
		return PE_RET_FAIL;
	}

	_stprintf(szHost,_T("http://%s:%d/%s"),s->szDevIP,PE_ANDROID_SVR_PORT,szFileName);

	return PE_RET_OK;
}



void CHomeUI::OnBnClickedBtnNewSms()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		MessageBox(_T("Device is not connected"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	CNewSmsDlg dlg;
	dlg.DoModal();
}

void CHomeUI::RestoreFromFolder(TCHAR *szFolderID)
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		MessageBox(_T("Device is not connected"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	CRestoreDlg *pDlg = new CRestoreDlg();
	pDlg->DoModal();
	g_iCanSync = 1;
	delete pDlg;
}

void CHomeUI::OnBnClickedBtnRestore()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		MessageBox(_T("Device is not connected"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	CRestoreDlg *pDlg = new CRestoreDlg();
	pDlg->DoModal();
	g_iCanSync = 1;
	delete pDlg;
}

void CHomeUI::OnBnClickedBtnAppManager()
{
	StartUI(PE_UI_TYPE_APP);
}

void CHomeUI::OnBnClickedBtnFile()
{
	StartUI(PE_UI_TYPE_FILE);

}

void CHomeUI::OnBnClickedBtnPhoto()
{
	StartUI(PE_UI_TYPE_PHOTO);

}

void CHomeUI::OnBnClickedBtnQuickStart()
{
	ShellExecute(0, _T("open"), _T("http://www.mobitnt.com/yasync-quick-start.htm"), NULL, NULL, SW_SHOWNORMAL);
}

