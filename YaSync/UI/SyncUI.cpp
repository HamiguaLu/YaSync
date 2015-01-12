// SyncUI.cpp : implementation file
//

#include "stdafx.h"
#include "PECore.h"

#include "YaSync.h"
#include "SyncUI.h"
#include "TrayHelper.h"
#include "SettingHelper.h"

// CSyncUI dialog

IMPLEMENT_DYNAMIC(CSyncUI, CDialog)

CSyncUI::CSyncUI(CWnd* pParent /*=NULL*/)
: CDialog(CSyncUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSyncUI::~CSyncUI()
{
}

void CSyncUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SYNC_PROGRESS, m_SyncProgress);
}


BEGIN_MESSAGE_MAP(CSyncUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_START_SYNC, &CSyncUI::OnBnClickedBtnStartSync)
	ON_BN_CLICKED(IDC_CHECK_SYNC_IMAGE, &CSyncUI::OnBnClickedCheckSyncImage)
	ON_BN_CLICKED(IDC_CHECK_SYNC_VIDEO, &CSyncUI::OnBnClickedCheckSyncVideo)
	ON_BN_CLICKED(IDC_CHECK_SYNC_AUDIO, &CSyncUI::OnBnClickedCheckSyncAudio)
END_MESSAGE_MAP()


// CSyncUI message handlers

HBRUSH CSyncUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CSyncUI::LoadSetting()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();

	CButton* pBtnSMS = (CButton*)GetDlgItem(IDC_CHECK_SYNC_SMS);
	CButton* pBtnCall = (CButton*)GetDlgItem(IDC_CHECK_SYNC_CALL);
	CButton* pBtnMMS = (CButton*)GetDlgItem(IDC_CHECK_SYNC_MMS);
	CButton* pBtnCalendar = (CButton*)GetDlgItem(IDC_CHECK_SYNC_CALENDAR);
	CButton* pBtnContacts = (CButton*)GetDlgItem(IDC_CHECK_SYNC_CONTACTS);
	
	CButton* pBtnImage = (CButton*)GetDlgItem(IDC_CHECK_SYNC_IMAGE);
	CButton* pBtnVideo = (CButton*)GetDlgItem(IDC_CHECK_SYNC_VIDEO);
	CButton* pBtnAudio = (CButton*)GetDlgItem(IDC_CHECK_SYNC_AUDIO);

	if (s)
	{
		pBtnSMS->SetCheck(s->dwSyncSMS);
		pBtnCall->SetCheck(s->dwSyncCall);
		pBtnMMS->SetCheck(s->dwSyncMMS);
		pBtnCalendar->SetCheck(s->dwSyncCalendar);
		pBtnContacts->SetCheck(s->dwSyncContact);
		pBtnImage->SetCheck(s->dwSyncImage);
		pBtnVideo->SetCheck(s->dwSyncVideo);
		pBtnAudio->SetCheck(s->dwSyncAudio);

		pBtnSMS->EnableWindow(1);
		pBtnCall->EnableWindow(1);
		pBtnMMS->EnableWindow(1);
		pBtnCalendar->EnableWindow(1);
		pBtnContacts->EnableWindow(1);

		pBtnImage->EnableWindow(1);
		pBtnVideo->EnableWindow(1);
		pBtnAudio->EnableWindow(1);
	}
	else
	{
		pBtnSMS->SetCheck(0);
		pBtnCall->SetCheck(0);
		pBtnMMS->SetCheck(0);
		pBtnCalendar->SetCheck(0);
		pBtnContacts->SetCheck(0);
		pBtnImage->SetCheck(0);
		pBtnVideo->SetCheck(0);
		pBtnAudio->SetCheck(0);

		pBtnSMS->EnableWindow(0);
		pBtnCall->EnableWindow(0);
		pBtnMMS->EnableWindow(0);
		pBtnCalendar->EnableWindow(0);
		pBtnContacts->EnableWindow(0);
		pBtnImage->EnableWindow(0);
		pBtnVideo->EnableWindow(0);
		pBtnAudio->EnableWindow(0);
	}
}

BOOL CSyncUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnStartSync.SubclassDlgItem(IDC_BTN_START_SYNC, this);
	m_btnStartSync.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnStartSync.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnStartSync.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnStartSync.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnStartSync.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnStartSync.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	m_SyncProgress.SetRange(0,100);

	LoadSetting();

	m_iSyncDone = 1;

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

extern int g_iCanSync;
int g_iCanceled = 0;
void CSyncUI::OnBnClickedBtnStartSync()
{
	if (m_iSyncDone)
	{
		PE_PHONE_SETTING* s = PECore::GetCurPhone();
		CButton* pBtnSMS = (CButton*)GetDlgItem(IDC_CHECK_SYNC_SMS);
		CButton* pBtnCall = (CButton*)GetDlgItem(IDC_CHECK_SYNC_CALL);
		CButton* pBtnMMS = (CButton*)GetDlgItem(IDC_CHECK_SYNC_MMS);
		CButton* pBtnCalendar = (CButton*)GetDlgItem(IDC_CHECK_SYNC_CALENDAR);
		CButton* pBtnContacts = (CButton*)GetDlgItem(IDC_CHECK_SYNC_CONTACTS);

		CButton* pBtnImage = (CButton*)GetDlgItem(IDC_CHECK_SYNC_IMAGE);
		CButton* pBtnVideo = (CButton*)GetDlgItem(IDC_CHECK_SYNC_VIDEO);
		CButton* pBtnAudio = (CButton*)GetDlgItem(IDC_CHECK_SYNC_AUDIO);

		if ( s)
		{
			s->dwSyncSMS = pBtnSMS->GetCheck();
			s->dwSyncCall = pBtnCall->GetCheck();
			s->dwSyncMMS = pBtnMMS->GetCheck();
			s->dwSyncCalendar = pBtnCalendar->GetCheck();
			s->dwSyncContact = pBtnContacts->GetCheck();
			s->dwSyncImage = pBtnImage->GetCheck();
			s->dwSyncVideo = pBtnVideo->GetCheck();
			s->dwSyncAudio = pBtnAudio->GetCheck();

			CSettingHelper::SetPhoneSetting(s);
		}

		LoadSetting();

		PECore::PostMessage(WM_SYNC_NOW,0,0);
		m_btnStartSync.SetWindowText(_T("Stop Sync"));
		m_iSyncDone = 0;
		g_iCanSync = 1;

		g_iCanceled = 0;
		m_SyncProgress.SetPos(0);
	}
	else
	{
		g_iCanSync = 0;
		g_iCanceled = 1;
		SetDlgItemText(IDC_STATIC_NOW_SYNC,_T("Canceling Sync process now..."));
	}
}


extern SYNC_STAT g_SyncStat;
int CSyncUI::OnSyncProcess(int iStart)
{
	if (iStart)
	{
		m_iSyncDone = 0;
		m_btnStartSync.SetWindowText(_T("Stop Sync"));
		SetDlgItemText(IDC_STATIC_NOW_SYNC,_T("Sync process is started now"));

		return 0;
	}

	m_iSyncDone = 1;
	m_btnStartSync.SetWindowText(_T("Start Sync"));


	SYSTEMTIME stLocal;	
	GetLocalTime(&stLocal);
	
	TCHAR szTime[255];
	_stprintf(szTime,_T("at  %0d-%02d  %02d:%02d:%02d"),stLocal.wMonth,stLocal.wDay,stLocal.wHour,stLocal.wMinute,stLocal.wSecond);
	
	CString sTime = _T("Sync done ");
	if (g_iCanceled)
	{
		sTime = _T("Sync canceled ");
	}
	sTime += szTime;

	SetDlgItemText(IDC_STATIC_NOW_SYNC,sTime);

	if (g_iCanceled)
	{
		return 0;
	}

	m_SyncProgress.SetPos(100);

	sTime += _T("\r\n");
	CString sStat;
	int iHaveData = 0;
	if (g_SyncStat.iSmsCount)
	{
		sStat.Format(_T("%d SMS"),g_SyncStat.iSmsCount);
		sTime += sStat;
		iHaveData = 1;
	}

	if (g_SyncStat.iMmsCount)
	{
		if (iHaveData)
		{
			sTime += _T(", ");
		}
		sStat.Format(_T("%d MMS"),g_SyncStat.iMmsCount);
		sTime += sStat;
	
		iHaveData = 1;
	}

	if (g_SyncStat.iCallCount)
	{
		if (iHaveData)
		{
			sTime += _T(", ");
		}

		sStat.Format(_T("%d Calls"),g_SyncStat.iCallCount);
		sTime += sStat;
		iHaveData = 1;
	}
	if (g_SyncStat.iContactsCount)
	{
		if (iHaveData)
		{
			sTime += _T(", ");
		}
		
		sStat.Format(_T("%d Contacts"),g_SyncStat.iContactsCount);
		sTime += sStat;
		iHaveData = 1;
	}
	if (g_SyncStat.iCalendarCount)
	{
		if (iHaveData)
		{
			sTime += _T(", ");
		}
		
		sStat.Format(_T("%d Calendars"),g_SyncStat.iCalendarCount);
		sTime += sStat;
		iHaveData = 1;
	}
	if (g_SyncStat.iImageCount)
	{
		if (iHaveData)
		{
			sTime += _T(", ");
		}
		
		sStat.Format(_T("%d Images"),g_SyncStat.iImageCount);
		sTime += sStat;
		iHaveData = 1;
	}
	if (g_SyncStat.iVideoCount)
	{
		if (iHaveData)
		{
			sTime += _T(", ");
		}
		
		sStat.Format(_T("%d Videos"),g_SyncStat.iVideoCount);
		sTime += sStat;
		iHaveData = 1;
	}
	if (g_SyncStat.iAudioCount)
	{
		if (iHaveData)
		{
			sTime += _T(", ");
		}
		
		sStat.Format(_T("%d Audios"),g_SyncStat.iAudioCount);
		sTime += sStat;
		iHaveData = 1;
	}

	if (iHaveData)
	{
		sTime += _T("  transfered to Outlook");
		SetDlgItemText(IDC_STATIC_NOW_SYNC,sTime);

	}


	return 0;
}

int CSyncUI::OnSyncInfo(PE_SYNC_STATUS *s)
{
	if (s->fTotalCount <= 0)
	{
		return PE_RET_FAIL;
	}

	float fPercent = (s->fCurCount/s->fTotalCount) * 100;
	m_SyncProgress.SetPos((int)fPercent);

	CString sPercent;
	sPercent.Format(_T(", %0.2f %% complete"),fPercent);

	CString sInfo = _T("Unkonw Sync Type");
	switch(s->iSyncType)
	{
	case PE_SYNC_SMS:
		{
			sInfo = _T("Now Sync SMS");
			break;
		}
	case PE_SYNC_CALL:
		{
			sInfo = _T("Now Sync Call Log");
			break;
		}
	case PE_SYNC_CONTACT:
		{
			sInfo = _T("Now Sync Contact");
			break;
		}
	case PE_SYNC_SIM_CONTACT:
		{
			sInfo = _T("Now Sync SIM Contact");
			break;
		}
	case PE_SYNC_CONTACT_2_PHONE:
		{
			sInfo = _T("Now Sync Contact From PC to Phone");
			break;
		}
	case PE_SYNC_MMS:
		{
			sInfo = _T("Now Sync MMS");
			break;
		}
	case PE_SYNC_CALENDAR:
		{
			sInfo = _T("Now Sync Calendar");
			break;
		}
	case PE_SYNC_CAL_2_PHONE:
		{
			sInfo = _T("Now Sync Calendar From PC to Phone");
			break;
		}
	case PE_SYNC_IMAGE:
		{
			sInfo = _T("Now Sync Image From Phone to PC");
			break;
		}
	case PE_SYNC_VIDEO:
		{
			sInfo = _T("Now Sync Video From Phone to PC");
			break;
		}
	case PE_SYNC_AUDIO:
		{
			sInfo = _T("Now Sync Audio From Phone to PC");
			break;
		}
	}

	sInfo += sPercent;

	SetDlgItemTextW(IDC_STATIC_NOW_SYNC,sInfo);

	return PE_RET_OK;
}

void CSyncUI::OnBnClickedCheckSyncImage()
{
	// TODO: Add your control notification handler code here
}

void CSyncUI::OnBnClickedCheckSyncVideo()
{
	// TODO: Add your control notification handler code here
}

void CSyncUI::OnBnClickedCheckSyncAudio()
{
	// TODO: Add your control notification handler code here
}
