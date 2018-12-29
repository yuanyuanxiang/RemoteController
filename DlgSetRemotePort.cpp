// DlgSetRemotePort.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "DlgSetRemotePort.h"
#include "afxdialogex.h"


// CDlgSetRemotePort 对话框

IMPLEMENT_DYNAMIC(CDlgSetRemotePort, CDialog)

CDlgSetRemotePort::CDlgSetRemotePort(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSetRemotePort::IDD, pParent)
	, m_nPort(0)
{

}

CDlgSetRemotePort::~CDlgSetRemotePort()
{
}

void CDlgSetRemotePort::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PORT, m_EditPort);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDV_MinMaxInt(pDX, m_nPort, 1, 65535);
}


BEGIN_MESSAGE_MAP(CDlgSetRemotePort, CDialog)
END_MESSAGE_MAP()


// CDlgSetRemotePort 消息处理程序
