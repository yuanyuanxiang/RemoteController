
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

	// 删除表格中的一行
	void DeleteAppItem(CSocketClient *client);

	// 更新表格中的一行
	void UpdateAppItem(CSocketClient *client, const AppInfo &it);

private:
	bool m_bExit;		/**< 是否退出程序 */
	bool m_bIsListen;	/**< 是否开启监听线程 */

	CRITICAL_SECTION m_cs;

	void Lock() { EnterCriticalSection(&m_cs); }

	void Unlock() { LeaveCriticalSection(&m_cs); }

	/// 只针对server端，监听数据
	int CheckIO();
	/// 检查是否有新连接(仅限server)
	static UINT WINAPI CheckIOThread(LPVOID param);

public:
	CSocketClient *g_fd_ArrayC[MAX_LISTEN];/**< server端处理所有的待决连接 */
};
