#pragma once
#include "afx.h"
#include "socket\SocketClient.h"
#include "AppInfo.h"
#include "cmdList.h"
#include <map>

// 用户自定义消息
enum UserMsg
{
	MSG_InsertApp = (WM_USER + 100), 
	MSG_UpdateApp, 
	MSG_DeleteApp, 
	MSG_ChangeColor, 
	MSG_Infomation,			// 收到消息
};

#define COLOR_DEFAULT 2047	// 默认字体颜色
#define COLOR_RED 2048		// 红色字体:程序异常或网络时延较大
#define COLOR_YELLOW 2049	// 黄色字体:窗口异常
#define COLOR_BLUE1 2050	// 蓝色字体:网络或校时异常
#define COLOR_BLUE2 2051	// 深蓝:时间差距较大

// 字符串
class String
{
private:
	int *ref;
	char *buf;
	String& operator = (const String &o) { return *this; }

public:
	explicit String(const int size) : ref(new int(1)), buf(new char[size+1])
	{
	}
	String(const String &o)
	{
		ref = o.ref;
		buf = o.buf;
		++(*ref);
	}
	~String()
	{
		if (0 == --(*ref))
		{
			delete ref;
			delete [] buf;
		}
	}
	operator char*() const { return buf; }
	operator const char*() const { return buf; }
	char& operator [] (int i) { return buf[i]; }
	const char* tolower() { return _strlwr(buf); }
	const char* c_str() const { return buf; }
};

// 替代W2A函数
String W_2_A(const CString &wStr);

#define MENU_COUNT 7

// CAppListCtrl

class CAppListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CAppListCtrl)

	int m_nIndex;		// 当前选中行

	std::map<int, int> m_MapStat; // 各个菜单状态

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

	// 监视所选程序
	void SpyOnSelected(const char *no, int nPort);

	// 清理资源、退出线程
	void Uninit_ffplay(int nPort);

	int GetUdpPort(int base = 5555);

	struct ffplayInfo
	{
		int udp_port;
		time_t t;
		ffplayInfo(int port) : udp_port(port), t(time(NULL)) { }
		bool timeout(int timeouttime) const { return time(NULL) - t > timeouttime; }
		operator int() const { return udp_port; }
	};

	std::map<std::string, ffplayInfo> m_ffplayMap;// 每个连接对应的ffplay端口

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
	afx_msg LRESULT MessageInfomation(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnOpRemote();
	afx_msg void OnUpdateOpRemote(CCmdUI *pCmdUI);
	afx_msg void OnOpSpy();
	afx_msg void OnUpdateOpSpy(CCmdUI *pCmdUI);
};
