#pragma once
#include "afx.h"
#include "socket\SocketClient.h"
#include "AppInfo.h"
#include "cmdList.h"

// CAppListCtrl

class CAppListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CAppListCtrl)

	int m_nIndex;		// 当前选中行

public:
	CAppListCtrl();
	virtual ~CAppListCtrl();

	// 增加列
	void AddColumns(const CString its[], int cols);

	// 插入行
	void InsertAppItem(const char* port);

	// 删除行
	void DeleteAppItem(const char* port);

	// 更新行
	void UpdateAppItem(const char* port, const AppInfo &it);

	// 平均分布各列
	void AvgColumnWidth(int cols);

	void Clear();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void RestartApp();
	afx_msg void QueryAppInfo();
	afx_msg void OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void StopApp();
	afx_msg void StartApp();
	afx_msg void UpdateApp();
};
