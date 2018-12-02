// NoticeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "NoticeDlg.h"
#include "afxdialogex.h"


// CNoticeDlg 对话框

IMPLEMENT_DYNAMIC(CNoticeDlg, CDialogEx)

CNoticeDlg::CNoticeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNoticeDlg::IDD, pParent)
	, m_strNotice(_T(""))
{

}

CNoticeDlg::~CNoticeDlg()
{
}

void CNoticeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NOTICE, m_strNotice);
}


BEGIN_MESSAGE_MAP(CNoticeDlg, CDialogEx)
END_MESSAGE_MAP()


// CNoticeDlg 消息处理程序


BOOL CNoticeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CEdit *pEdit = (CEdit*)GetDlgItem(IDC_EDIT_NOTICE);
	// 最多输入100个字
	pEdit->SetLimitText(100);
	return TRUE;
}
