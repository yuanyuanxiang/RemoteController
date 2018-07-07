#pragma once
#include "afx.h"
#include "socket\SocketClient.h"
#include "AppInfo.h"
#include "cmdList.h"

// 用户自定义消息
enum UserMsg
{
	MSG_InsertApp = (WM_USER + 100), 
	MSG_UpdateApp, 
	MSG_DeleteApp, 
	MSG_ChangeColor, 
};

#define COLOR_DEFAULT 2047	// 默认字体颜色
#define COLOR_RED 2048		// 红色字体:程序异常
#define COLOR_YELLOW 2049	// 黄色字体:窗口异常
#define COLOR_BLUE1 2050	// 蓝色字体:网络或校时异常
#define COLOR_BLUE2 2051	// 深蓝

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

	inline void Lock() { EnterCriticalSection(&m_cs); }

	inline void Unlock() { LeaveCriticalSection(&m_cs); }

	std::string getCurSelNo();

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
	afx_msg LRESULT MessageInsertApp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MessageUpdateApp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MessageDeleteApp(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT MessageChangeColor(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};
