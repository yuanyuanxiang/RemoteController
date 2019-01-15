#pragma once
#include "afxwin.h"


// CDlgSetRemoteIP 对话框

/************************************************************************
* @class CDlgSetRemoteIP
* @brief 设置守护程序远程IP
/************************************************************************/
class CDlgSetRemoteIP : public CDialog
{
	DECLARE_DYNAMIC(CDlgSetRemoteIP)

public:
	CDlgSetRemoteIP(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSetRemoteIP();

// 对话框数据
	enum { IDD = IDD_SET_REMOTEIP_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditIP;
	CString m_strIP;
};
