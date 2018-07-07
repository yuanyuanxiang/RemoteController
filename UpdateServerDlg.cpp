// UpdateServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "UpdateServerDlg.h"
#include "afxdialogex.h"


// CUpdateServerDlg 对话框

IMPLEMENT_DYNAMIC(CUpdateServerDlg, CDialogEx)

CUpdateServerDlg::CUpdateServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUpdateServerDlg::IDD, pParent)
	, m_strUpServer(_T(""))
{

}

CUpdateServerDlg::~CUpdateServerDlg()
{
}

void CUpdateServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SERVER, m_EditUpServer);
	DDX_Text(pDX, IDC_EDIT_SERVER, m_strUpServer);
	DDV_MaxChars(pDX, m_strUpServer, 52);
}


BEGIN_MESSAGE_MAP(CUpdateServerDlg, CDialogEx)
END_MESSAGE_MAP()


// CUpdateServerDlg 消息处理程序
