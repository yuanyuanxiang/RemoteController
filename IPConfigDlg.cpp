// IPConfigDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "IPConfigDlg.h"
#include "afxdialogex.h"


// CIPConfigDlg 对话框

IMPLEMENT_DYNAMIC(CIPConfigDlg, CDialog)

CIPConfigDlg::CIPConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIPConfigDlg::IDD, pParent)
	, m_strIpAddr(_T(""))
	, m_nPort(9999)
{

}

CIPConfigDlg::~CIPConfigDlg()
{
}

void CIPConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_IPADDR, m_IpAddr);
	DDX_Control(pDX, IDC_EDIT_IPPORT, m_Port);
	DDX_Text(pDX, IDC_EDIT_IPADDR, m_strIpAddr);
	DDX_Text(pDX, IDC_EDIT_IPPORT, m_nPort);
	DDV_MinMaxInt(pDX, m_nPort, 1, 65535);
}


BEGIN_MESSAGE_MAP(CIPConfigDlg, CDialog)
END_MESSAGE_MAP()


// CIPConfigDlg 消息处理程序
