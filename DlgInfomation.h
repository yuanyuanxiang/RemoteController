#pragma once
#include "afxwin.h"

/************************************************************************
* @brief CDlgInfomation 对话框
* @details 倒计时关闭对话框
************************************************************************/
class CDlgInfomation : public CDialog
{
	DECLARE_DYNAMIC(CDlgInfomation)

public:
	CDlgInfomation(const char *info, const char *details, int tm, CWnd* pParent = NULL);
	virtual ~CDlgInfomation();

// 对话框数据
	enum { IDD = IDD_INFOMATION_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	int m_nTime;			// ms
	CString m_strInfo;
	CString m_strTitle;
	CStatic m_StaticInfo;

	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
