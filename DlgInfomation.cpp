// DlgInfomation.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "DlgInfomation.h"
#include "afxdialogex.h"

#define AUTO_KILL 0

// CDlgInfomation 对话框

IMPLEMENT_DYNAMIC(CDlgInfomation, CDialog)

CDlgInfomation::CDlgInfomation(const char *info, const char *details, int tm, CWnd* pParent)
	: CDialog(CDlgInfomation::IDD, pParent)
	, m_strTitle(info)
	, m_strInfo(details)
	, m_nTime(max(tm, 3333))
{
}


CDlgInfomation::~CDlgInfomation()
{
}

void CDlgInfomation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDV_MaxChars(pDX, m_strInfo, 2048);
	DDX_Text(pDX, IDC_INFOMATION, m_strInfo);
	DDX_Control(pDX, IDC_INFOMATION, m_StaticInfo);
}


BEGIN_MESSAGE_MAP(CDlgInfomation, CDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDlgInfomation 消息处理程序


BOOL CDlgInfomation::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetTimer(AUTO_KILL, m_nTime, NULL);

	SetWindowText(m_strTitle);
	m_StaticInfo.SetWindowText(m_strInfo);

	return TRUE;
}


void CDlgInfomation::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == AUTO_KILL)
	{
		KillTimer(AUTO_KILL);
		SendMessage(WM_CLOSE, 0, 0);
		return;
	}

	CDialog::OnTimer(nIDEvent);
}
