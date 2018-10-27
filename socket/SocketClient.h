#pragma once

/** Copyright notice \n
* Copyright (c) 2018, BCECR
* All rights reserved.
* @file		SocketClient.h
* @brief	BCECR SOCKET CLIENT
* @version	1.0
* @date		2018/3/20
* @update	
* @author	yuanyuanxiang
*/

#include "SocketBase.h"
#include "RingBuffer.h"
#include "tinyxml\tinyxml.h"
#include <vector>
#include "..\AppInfo.h"

/** 
* @class	CSocketServer 
* @brief	socket通信类客户端
* @details	实现基本的收/发数据的功能
*/
class CSocketClient : public CSocketBase
{
private:
	char m_strSrcPort[64];	// 源端口
	char m_strName[64];		// 名称
	int m_nSrcPort;			// 源端口
	bool m_bExit;			// 是否退出
	bool m_bAlive;			// 是否保持着连接
	bool m_bIsParsing;		// 解析线程是否开启
	bool m_bIsReceiving;	// 收数据线程是否开启
	int m_nAliveTime;		// 心跳周期（秒）
	clock_t m_tick;			// 消息一个来回的计时器

	char *m_RecvBuffer;			// 收数据缓存
	RingBuffer *m_RingBuffer;	// 缓存区
	TiXmlDocument *m_xmlParser;
	AppInfo item;				// AppInfo

	static UINT WINAPI ParseThread(LPVOID param);
	static UINT WINAPI ReceiveThread(LPVOID param);

	/// 收MU消息
	void ReceiveData(const char *buffer, int nLength)
	{
		if(*buffer)
			m_RingBuffer->PushData(buffer, nLength);
	}

	/// 解析MU消息
	void ParseData()
	{
		if (m_RingBuffer->GetXml(m_RecvBuffer, &m_bExit))
			ReadSipXmlInfo(m_RecvBuffer, m_RingBuffer->GetSipXmlLength());
	}

	// 读取SipXml
	void ReadSipXmlInfo(const char *buffer, int nLen);

protected:
	~CSocketClient(void); // 只能通过new产生本类对象

public:
	CSocketClient(SOCKET client, const char *Ip, int port);

	void Destroy() { delete this; }

	/// socket退出时进行清理工作
	void unInit();

	// 设置心跳周期
	void SetAliveTime(int nAliveTime) { m_nAliveTime = nAliveTime; }

	// 记录当前时间
	void StartClock() { m_tick = clock(); }

	// 标记为将退出
	void SetExit() { m_bExit = true; m_bAlive = false; }

	/// 获取编号
	const char* GetNo() const { return m_strSrcPort; }
	int GetSrcPort() const { return m_nSrcPort; }

	/// 获取名称
	const char* Name() const { return m_strName; }

	/// 获取IP
	const char* GetIp() const { return m_chToIp; }

	/// 是否使用
	bool IsAlive() const { return m_bAlive; }

	// 断开连接
	void Disconnect();
};
