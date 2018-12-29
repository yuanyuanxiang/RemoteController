// DlgSetRemoteIP.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "DlgSetRemoteIP.h"
#include "afxdialogex.h"


// CDlgSetRemoteIP 对话框

IMPLEMENT_DYNAMIC(CDlgSetRemoteIP, CDialog)

CDlgSetRemoteIP::CDlgSetRemoteIP(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSetRemoteIP::IDD, pParent)
	, m_strIP(_T(""))
{

}

CDlgSetRemoteIP::~CDlgSetRemoteIP()
{
}

void CDlgSetRemoteIP::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_IP, m_EditIP);
	DDX_Text(pDX, IDC_EDIT_IP, m_strIP);
	DDV_MaxChars(pDX, m_strIP, 16);
}


BEGIN_MESSAGE_MAP(CDlgSetRemoteIP, CDialog)
END_MESSAGE_MAP()


// CDlgSetRemoteIP 消息处理程序
