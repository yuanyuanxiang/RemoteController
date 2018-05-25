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

	// 初始化所有列
	void AddColumns(const CString its[], int cols);

	// 平均分布各列
	void AvgColumnWidth(int cols);

	//////////////////////////////////////////////////////////////////////////
	// 以下4个函数涉及多线程

	// 插入行[m]
	void InsertAppItem(const char* port);

	// 删除行[m]
	void DeleteAppItem(const char* port);

	// 更新行[m]
	void UpdateAppItem(const char* port, const AppInfo &it);

	// 清空所有行[m]
	void Clear();

	CRITICAL_SECTION m_cs;

	void Lock() { EnterCriticalSection(&m_cs); }

	void Unlock() { LeaveCriticalSection(&m_cs); }

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
