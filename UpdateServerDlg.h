#pragma once
#include "afxwin.h"


// CUpdateServerDlg 对话框

class CUpdateServerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CUpdateServerDlg)

public:
	CUpdateServerDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CUpdateServerDlg();

// 对话框数据
	enum { IDD = IDD_UPSERVER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditUpServer;
	CString m_strUpServer;
};
