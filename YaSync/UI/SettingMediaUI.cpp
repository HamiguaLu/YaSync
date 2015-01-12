// SettingMediaUI.cpp : implementation file
//

#include "stdafx.h"
#include "SettingMediaUI.h"
#include "PECore.h"

// CSettingMediaUI dialog

IMPLEMENT_DYNAMIC(CSettingMediaUI, CDialog)
extern HWND g_hSettingUI;

CSettingMediaUI::CSettingMediaUI(CWnd* pParent /*=NULL*/)
: CDialog(CSettingMediaUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingMediaUI::~CSettingMediaUI()
{
}

void CSettingMediaUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSettingMediaUI, CDialog)
	ON_BN_CLICKED(IDC_BTN_GET_IMAGE_PATH, &CSettingMediaUI::OnBnClickedBtnGetImagePath)
	ON_BN_CLICKED(IDC_BTN_GET_VIDEO_PATH, &CSettingMediaUI::OnBnClickedBtnGetVideoPath)
	ON_BN_CLICKED(IDC_BTN_GET_AUDIO_PATH, &CSettingMediaUI::OnBnClickedBtnGetAudioPath)
	ON_BN_CLICKED(IDC_ENABLE_IMAGE_SYNC, &CSettingMediaUI::OnBnClickedEnableImageSync)
	ON_BN_CLICKED(IDC_ENABLE_VIDEO_SYNC, &CSettingMediaUI::OnBnClickedEnableVideoSync)
	ON_BN_CLICKED(IDC_ENABLE_AUDIO_SYNC, &CSettingMediaUI::OnBnClickedEnableAudioSync)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()



void CSettingMediaUI::OnDeviceConnected()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_IMAGE_SYNC);
	pBtn->SetCheck(s->dwSyncImage);
	pBtn = (CButton*)GetDlgItem(IDC_ENABLE_VIDEO_SYNC);
	pBtn->SetCheck(s->dwSyncVideo);
	pBtn = (CButton*)GetDlgItem(IDC_ENABLE_AUDIO_SYNC);
	pBtn->SetCheck(s->dwSyncAudio);

	SetDlgItemText(IDC_EDIT_IMAGE_FOLDER_PATH,s->szImageFolderName);
	SetDlgItemText(IDC_EDIT_VIDEO_FODLER_PATH,s->szVideoFolderName);
	SetDlgItemText(IDC_EDIT_AUDIO_FOLDER_PATH,s->szAudioFolderName);
}

void CSettingMediaUI::OnSave()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CString sPath;
	CButton* pBtn = NULL;
	pBtn = (CButton*)GetDlgItem(IDC_ENABLE_IMAGE_SYNC);
	sPath = _T("");
	GetDlgItemText(IDC_EDIT_IMAGE_FOLDER_PATH,sPath);
	if (pBtn->GetCheck() && sPath.GetLength() < 1)
	{
		MessageBox(_T("Please choose the image folder"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}
	s->dwSyncImage = pBtn->GetCheck();
	_tcscpy(s->szImageFolderName,sPath);

	pBtn = (CButton*)GetDlgItem(IDC_ENABLE_VIDEO_SYNC);
	sPath = _T("");
	GetDlgItemText(IDC_EDIT_IMAGE_FOLDER_PATH,sPath);
	if (pBtn->GetCheck() && sPath.GetLength() < 1)
	{
		MessageBox(_T("Please choose the video folder"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}
	s->dwSyncVideo = pBtn->GetCheck();
	_tcscpy(s->szVideoFolderName,sPath);

	pBtn = (CButton*)GetDlgItem(IDC_ENABLE_AUDIO_SYNC);
	sPath = _T("");
	GetDlgItemText(IDC_EDIT_IMAGE_FOLDER_PATH,sPath);
	if (pBtn->GetCheck() && sPath.GetLength() < 1)
	{
		MessageBox(_T("Please choose the audio folder"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}
	s->dwSyncAudio = pBtn->GetCheck();
	_tcscpy(s->szAudioFolderName,sPath);

}




// CSettingMediaUI message handlers

void CSettingMediaUI::OnBnClickedBtnGetImagePath()
{
	TCHAR pszPath[MAX_PATH];  
	BROWSEINFO bi;   
	bi.hwndOwner      = this->GetSafeHwnd();  
	bi.pidlRoot       = NULL;  
	bi.pszDisplayName = NULL;   
	bi.lpszTitle      = TEXT("Please choose folder");   
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;  
	bi.lpfn           = NULL;   
	bi.lParam         = 0;  
	bi.iImage         = 0;   

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);  
	if (pidl == NULL)  
	{  
		return;  
	}  

	if (SHGetPathFromIDList(pidl, pszPath))  
	{  
		SetDlgItemText(IDC_EDIT_IMAGE_FOLDER_PATH,pszPath);
		::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,10,0);
	} 

}

void CSettingMediaUI::OnBnClickedBtnGetVideoPath()
{
	TCHAR pszPath[MAX_PATH];  
	BROWSEINFO bi;   
	bi.hwndOwner      = this->GetSafeHwnd();  
	bi.pidlRoot       = NULL;  
	bi.pszDisplayName = NULL;   
	bi.lpszTitle      = TEXT("Please choose folder");   
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;  
	bi.lpfn           = NULL;   
	bi.lParam         = 0;  
	bi.iImage         = 0;   

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);  
	if (pidl == NULL)  
	{  
		return;  
	}  

	if (SHGetPathFromIDList(pidl, pszPath))  
	{  
		SetDlgItemText(IDC_EDIT_VIDEO_FODLER_PATH,pszPath);
		::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,10,0);
	} 

}

void CSettingMediaUI::OnBnClickedBtnGetAudioPath()
{
	TCHAR pszPath[MAX_PATH];  
	BROWSEINFO bi;   
	bi.hwndOwner      = this->GetSafeHwnd();  
	bi.pidlRoot       = NULL;  
	bi.pszDisplayName = NULL;   
	bi.lpszTitle      = TEXT("Please choose folder");   
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;  
	bi.lpfn           = NULL;   
	bi.lParam         = 0;  
	bi.iImage         = 0;   

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);  
	if (pidl == NULL)  
	{  
		return;  
	}  

	if (SHGetPathFromIDList(pidl, pszPath))  
	{  
		SetDlgItemText(IDC_EDIT_AUDIO_FOLDER_PATH,pszPath);
		::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,10,0);
	} 
}

void CSettingMediaUI::OnBnClickedEnableImageSync()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,10,0);
}

void CSettingMediaUI::OnBnClickedEnableVideoSync()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,10,0);
}

void CSettingMediaUI::OnBnClickedEnableAudioSync()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,10,0);
}

HBRUSH CSettingMediaUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CSettingMediaUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnImagePath.SubclassDlgItem(IDC_BTN_GET_IMAGE_PATH, this);
	m_btnImagePath.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnImagePath.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnImagePath.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnImagePath.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnImagePath.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnImagePath.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	m_btnVideoPath.SubclassDlgItem(IDC_BTN_GET_VIDEO_PATH, this);
	m_btnVideoPath.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnVideoPath.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnVideoPath.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnVideoPath.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnVideoPath.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnVideoPath.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	m_btnAudioPath.SubclassDlgItem(IDC_BTN_GET_AUDIO_PATH, this);
	m_btnAudioPath.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnAudioPath.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnAudioPath.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnAudioPath.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnAudioPath.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnAudioPath.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


