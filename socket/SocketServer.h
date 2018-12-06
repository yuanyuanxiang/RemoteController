
/** Copyright notice \n
* Copyright (c) 2018, BCECR
* All rights reserved.
* @file		SocketServer.h
* @brief	BCECR SOCKET SERVER
* @version	1.0
* @date		2018/3/20
* @update	
* @author	yuanyuanxiang
*/

#pragma once

#include "SocketBase.h"
#include <map>
#include "..\AppInfo.h"

class CSocketClient;

// 错误码对应的错误信息
extern const char *pErrorCode[];

/** 
* @class	CSocketServer 
* @brief	socket通信类服务端
* @details	实现基本的收/发数据的功能
*/
class CSocketServer : public CSocketBase
{
public:
	/// 构造函数
	CSocketServer();
	/// 析构
	~CSocketServer();
	/// 初始化socket
	int init(const char *pIp, int nPort, int nType); //0:server, 1:client
	/// socket退出时进行清理工作
	void unInit();

	// 向APP发送消息[id==NULL表示全部]
	void SendCommand(const char *msg, const char *id = NULL);

	// 控制硬件设备：关闭/重启/时间同步
	void ControlDevice(const char *msg);

	// 设置心跳周期（即刷新频率）
	void SetAliveTime(int msg, const char *id = NULL);

	CRITICAL_SECTION m_cs;

	void Lock() { EnterCriticalSection(&m_cs); }

	void Unlock() { LeaveCriticalSection(&m_cs); }

private:
	bool m_bExit;		/**< 是否退出程序 */
	bool m_bIsListen;	/**< 是否开启监听线程 */

	/// 只针对server端，监听数据
	int CheckIO();
	// 获取可用的Client
	int GetAvailabeClient();
	/// 检查是否有新连接(仅限server)
	static UINT WINAPI CheckIOThread(LPVOID param);
	/// 当前目录下所有程序的版本集合
	std::map<std::string, std::string> m_mapVersion;
	void GetAllSoftwareVersion();

public:
	// 检测是否需要更新版本，若需要则返回新版本号
	std::string CheckUpdate(const char *app) const;

public:
	CSocketClient *g_fd_ArrayC[MAX_LISTEN];/**< server端处理所有的待决连接 */
};
