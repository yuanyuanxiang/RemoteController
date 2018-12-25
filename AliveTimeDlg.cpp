// AliveTimeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "AliveTimeDlg.h"
#include "afxdialogex.h"


// CAliveTimeDlg 对话框

IMPLEMENT_DYNAMIC(CAliveTimeDlg, CDialog)

CAliveTimeDlg::CAliveTimeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAliveTimeDlg::IDD, pParent)
	, m_nAliveTime(0)
{

}

CAliveTimeDlg::~CAliveTimeDlg()
{
}

void CAliveTimeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ALIVETIME, m_EditAliveTime);
	DDX_Text(pDX, IDC_EDIT_ALIVETIME, m_nAliveTime);
	DDV_MinMaxInt(pDX, m_nAliveTime, 1, 3600);
}


BEGIN_MESSAGE_MAP(CAliveTimeDlg, CDialog)
END_MESSAGE_MAP()


// CAliveTimeDlg 消息处理程序
