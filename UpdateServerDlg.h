#pragma once
#include "afxwin.h"


// CUpdateServerDlg 对话框

/************************************************************************
* @class CUpdateServerDlg
* @brief 设置升级服务器地址对话框（如果为端口，则采用socket，否则为IIS）
/************************************************************************/
class CUpdateServerDlg : public CDialog
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
