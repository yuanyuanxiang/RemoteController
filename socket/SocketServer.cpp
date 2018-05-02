#include <stdio.h>
#include <iostream>
#include "SocketServer.h"   
#include "SocketClient.h"

#ifndef USING_LOG
#define MY_LOG(pStr)	std::cout << pStr << std::endl
#else 
#include "../log.h"
#endif
#include <stdlib.h>
#include <process.h>

#include "..\AppListCtrl.h"

extern CAppListCtrl *g_pList;

// CheckIO线程[该线程最多只会启动一次]
UINT WINAPI CSocketServer::CheckIOThread(LPVOID param)
{
	CSocketServer *pThis = (CSocketServer*)param;
	if (pThis->m_bIsListen)
		return -1;
	pThis->m_bIsListen = true;
	OutputDebugStringA("CSocketServer CheckIOThread Start.\n");
	while (!pThis->m_bExit)
	{
		Sleep(10);
		pThis->CheckIO();
	}
	pThis->m_bIsListen = false;
	OutputDebugStringA("CSocketServer CheckIOThread Stop.\n");
	return 0xDead666;
}

/**
* @brief 默认的构造函数
*/
CSocketServer::CSocketServer()
{
	m_bExit = false;
	m_bIsListen = false;

	memset(g_fd_ArrayC, 0, sizeof(g_fd_ArrayC));
	InitializeCriticalSection(&m_cs);
}

/**
* @brief ~CSocketServer
*/
CSocketServer::~CSocketServer()
{
	DeleteCriticalSection(&m_cs);
}

// 在调用完基类的初始化函数之后，Server开启监听Client连接请求的线程
int CSocketServer::init(const char *pIp, int nPort, int nType)
{
	m_bExit = false;
	int ret = CSocketBase::init(pIp, nPort, nType);
	if(0 == ret)
		// 开始监听
		_beginthreadex(NULL, 0, &CheckIOThread, this, 0, NULL);
	return ret;
}


// 反初始化过程：先退出本类线程，清理本类内存，然后调用基类unInit函数
void CSocketServer::unInit()
{
	// 退出CheckIO
	Lock();
	m_bExit = true;
	Unlock();
	while (m_bIsListen)
		Sleep(10);

	Lock();
	for (int i = 0; i < MAX_LISTEN; ++i)
	{
		if (g_fd_ArrayC[i])
		{
			g_fd_ArrayC[i]->SetExit();
		}
	}
	for (int i = 0; i < MAX_LISTEN; ++i)
	{
		if (g_fd_ArrayC[i])
		{
			g_fd_ArrayC[i]->unInit();
			g_fd_ArrayC[i]->Destroy();
			g_fd_ArrayC[i] = NULL;
		}
	}
	Unlock();

	CSocketBase::unInit();
}


void CSocketServer::SendCommand(const char *msg, const char *id)
{
	TRACE("======> SendMessage[%s]: %s\n", id ? id : "ALL", msg);
	Lock();
	for (int i = 0; i < MAX_LISTEN; ++i)
	{
		if (g_fd_ArrayC[i])
		{
			if (id)
			{
				if(0 == strcmp(id, g_fd_ArrayC[i]->GetNo()))
				{
					g_fd_ArrayC[i]->sendData(msg, strlen(msg));
					break;
				}
			}else
			{
				g_fd_ArrayC[i]->sendData(msg, strlen(msg));
			}
		}
	}
	Unlock();
}


void CSocketServer::DeleteAppItem(CSocketClient *client)
{
	Lock();
	if (!m_bExit)
		g_pList->DeleteAppItem(client);
	client->SetExit();
	Unlock();
}


void CSocketServer::UpdateAppItem(CSocketClient *client, const AppInfo &it)
{
	Lock();
	if (!m_bExit)
		g_pList->UpdateAppItem(client, it);
	Unlock();
}


/** 
* @brief 只针对server端，监听数据
* @return 返回0为成功，其他为失败
*/
int CSocketServer::CheckIO()
{
	assert(0 == m_nType);
	timeval tv = {1, 0};
	FD_ZERO(&fdRead);
	FD_SET(m_SocketListen, &fdRead);
	/// 调用select模式进行监听
	int nRes = select(0, &fdRead, NULL, NULL, &tv);
	if (0 == nRes)
	{
		/// 超时，继续
		return _NO__ERROR;
	}
	else if (nRes < 0)
	{
		char str[DEFAULT_BUFFER];
		sprintf_s(str, "CSocketServer select failed, code = %d.\n", WSAGetLastError());
		MY_LOG(str);
		return ERROR_SELECT;
	}
	sockaddr_in addrClient;
	int addrClientLen = sizeof(addrClient);
	/// 检查是否有新的连接进入
	if (FD_ISSET(m_SocketListen, &fdRead))
	{
		m_Socket = accept(m_SocketListen, (sockaddr*)&addrClient, &addrClientLen);
		if (m_Socket == WSAEWOULDBLOCK)
		{
			MY_LOG("非阻塞模式设定accept调用不正确.\n");
			return ERROR_ACCEPT;
		}
		else if (m_Socket == INVALID_SOCKET)
		{
			char str[DEFAULT_BUFFER];
			sprintf_s(str, "CSocketServer accept failed, code = %d.\n", WSAGetLastError());
			MY_LOG(str);
			return ERROR_ACCEPT;
		}
		bool bFull = false;
		/// 新的连接可以使用，查看待决处理队列
		Lock();
		for (int nLoopi = 0; nLoopi < MAX_LISTEN; ++nLoopi)
		{
			if (0 == g_fd_ArrayC[nLoopi] || !g_fd_ArrayC[nLoopi]->IsAlive())
			{
				/// 添加新的可用连接
				bFull = false;
				if (g_fd_ArrayC[nLoopi])
				{
					g_pList->DeleteAppItem(g_fd_ArrayC[nLoopi]);
					g_fd_ArrayC[nLoopi]->unInit();
					g_fd_ArrayC[nLoopi]->Destroy();
					g_fd_ArrayC[nLoopi] = NULL;
				}
				g_fd_ArrayC[nLoopi] = new CSocketClient(m_Socket, 
					inet_ntoa(addrClient.sin_addr), ntohs(addrClient.sin_port));
				g_pList->InsertAppItem(g_fd_ArrayC[nLoopi]);
				break;
			}
		}
		Unlock();
		if (bFull)
		{
			char str[128];
			sprintf(str, "CSocketServer服务器端连接数已满（未接受%d）.\n", m_Socket);
			MY_LOG(str);
			closesocket(m_Socket);
			return ERROR_CLIENTFULL;
		}
	}//if (FD_ISSET(sListen, &fdRead))
	return _NO__ERROR;
}
