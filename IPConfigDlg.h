#pragma once
#include "afxwin.h"


// CIPConfigDlg 对话框

class CIPConfigDlg : public CDialog
{
	DECLARE_DYNAMIC(CIPConfigDlg)

public:
	CIPConfigDlg(CString sTitle = _T("监听设置"), CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CIPConfigDlg();

// 对话框数据
	enum { IDD = IDD_IPCONFIG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_IpAddr;
	CEdit m_Port;
	CString m_strIpAddr;
	CString m_strTitle;
	BOOL m_bModIP;
	int m_nPort;
	virtual BOOL OnInitDialog();
};
