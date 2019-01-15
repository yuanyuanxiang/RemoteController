#pragma once
#include "afxwin.h"


// CDlgSetRemotePort 对话框

/************************************************************************
* @class CDlgSetRemotePort
* @brief 设置守护程序连接的端口
/************************************************************************/
class CDlgSetRemotePort : public CDialog
{
	DECLARE_DYNAMIC(CDlgSetRemotePort)

public:
	CDlgSetRemotePort(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSetRemotePort();

// 对话框数据
	enum { IDD = IDD_SET_REMOTEPORT_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditPort;
	int m_nPort;
};
