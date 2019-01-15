#pragma once

#include "resource.h"

// CNoticeDlg 对话框

/************************************************************************
* @class CNoticeDlg
* @brief 提示信息窗口
/************************************************************************/
class CNoticeDlg : public CDialog
{
	DECLARE_DYNAMIC(CNoticeDlg)

public:
	CNoticeDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNoticeDlg();

// 对话框数据
	enum { IDD = IDD_NOTICE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_strNotice;
	virtual BOOL OnInitDialog();
};
