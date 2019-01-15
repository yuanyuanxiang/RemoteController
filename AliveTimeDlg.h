#pragma once
#include "afxwin.h"


// CAliveTimeDlg 对话框

/************************************************************************
* @class CAliveTimeDlg
* @brief 设置守护程序心跳周期（刷新频率）
/************************************************************************/
class CAliveTimeDlg : public CDialog
{
	DECLARE_DYNAMIC(CAliveTimeDlg)

public:
	CAliveTimeDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAliveTimeDlg();

// 对话框数据
	enum { IDD = IDD_REFRESH_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditAliveTime;
	int m_nAliveTime;
};
