#include "UpdateServer.h"
#include <stdio.h>
#include <io.h>
#include "md5driver.hpp"

#define SLEEP_TIME 50

folder g_f; // 目录访问器，仅单线程操作，多线程不安全

UpdateServer::UpdateServer(void)
{
	memset(m_strIp, 0, sizeof(m_strIp));
	m_nPort = 0;
	m_Socket = INVALID_SOCKET;
	m_AcceptThread = false;
	m_RecvDataThread = false;
	m_bExit = false;
	nConnNum = 0;
	::InitializeCriticalSection(&m_cs);
}


UpdateServer::~UpdateServer(void)
{
	::DeleteCriticalSection(&m_cs);
}

int UpdateServer::init(const char *pIp, int nPort)
{
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (m_Socket == INVALID_SOCKET)
		return -1;

	// 发送缓冲区
	const int nSendBuf = 1024 * 1024 * 2;
	::setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));
	// 接收缓冲区
	const int nRecvBuf = 1024 * 1024 * 2;
	::setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));
	bool bConditionalAccept = true;
	::setsockopt(m_Socket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (const char *)&bConditionalAccept, sizeof(bool));

	sockaddr_in m_in;
	m_in.sin_family = AF_INET;
	m_in.sin_addr.S_un.S_addr = inet_addr(pIp);
	strcpy_s(m_strIp, pIp);
	m_in.sin_port = htons(nPort);
	// 绑定端口
	if (SOCKET_ERROR == bind(m_Socket,(const sockaddr*)&m_in, sizeof(m_in)) )
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		char buf[256];
		sprintf_s(buf, "======> 绑定端口[%d]失败, GetLastError= %d\n", nPort, WSAGetLastError());
		OutputDebugStringA(buf);
		return -2;
	}
	// 监听端口
	if (SOCKET_ERROR == listen(m_Socket, MAX_CONNECT))
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		char buf[256];
		sprintf_s(buf, "======> 监听端口[%d]失败, GetLastError= %d\n", nPort, WSAGetLastError());
		OutputDebugStringA(buf);
		return -3;
	}
	// 非阻塞模式设定
	ULONG ul = 1;
	if (SOCKET_ERROR == ioctlsocket(m_Socket, FIONBIO, &ul))
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
		char buf[256];
		sprintf_s(buf, "======> 非阻塞模式设置失败, GetLastError= %d\n", WSAGetLastError());
		OutputDebugStringA(buf);
		return -4;
	}
	m_bExit = false;
	HANDLE h = 0;
	m_AcceptThread = h = (HANDLE)_beginthreadex(0, 0, AcceptThread, this, 0, 0);
	CloseHandle(h);
	m_RecvDataThread = h = (HANDLE)_beginthreadex(0, 0, RecvDataThread, this, 0, 0);
	CloseHandle(h);
	return 0;
}

void UpdateServer::unInit()
{
	m_bExit = true;
	while (m_AcceptThread || m_RecvDataThread)
		Sleep(10);
	for (int i = 0; i < MAX_CONNECT; ++i)
	{
		if (g_queue[i].s != INVALID_SOCKET)
		{
			g_queue[i].exit();
		}
	}
	if (INVALID_SOCKET != m_Socket)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
}

UINT WINAPI UpdateServer::AcceptThread(void *param)
{
	OutputDebugStringA("===> AcceptThread BEGIN\n");
	UpdateServer *pThis = (UpdateServer*)param;
	pThis->CheckIO();
	OutputDebugStringA("===> AcceptThread END\n");
	return 0xDEAD777;
}

void UpdateServer::CheckIO()
{
	fd_set fd;
	const timeval tv = {2, 0};
	while (!m_bExit)
	{
		FD_ZERO(&fd);
		FD_SET(m_Socket, &fd);
		int nRes = select(0, &fd, NULL, NULL, &tv);
		if (0 == nRes)
		{
			Sleep(SLEEP_TIME);
			continue;
		}
		else if (nRes < 0)
			break;
		if (m_bExit)
			break;

		sockaddr_in addrClient;
		int addrClientLen = sizeof(addrClient);
		if (FD_ISSET(m_Socket, &fd))
		{
			SOCKET s = accept(m_Socket, (sockaddr*)&addrClient, &addrClientLen);
			if (s == WSAEWOULDBLOCK)
			{
				Sleep(SLEEP_TIME);
				continue;
			}
			else if (s == INVALID_SOCKET)
			{
				Sleep(SLEEP_TIME);
				continue;
			}
			if (add_client(s))
			{
				const char *ip = inet_ntoa(addrClient.sin_addr);
				int port = ntohs(addrClient.sin_port);
				char buf[128];
				sprintf_s(buf, "===> New Client Accept: %s:%d\n", ip, port);
				OutputDebugStringA(buf);
			}else
			{
				closesocket(s);
				s = INVALID_SOCKET;
				Sleep(1000);
			}
		}
	}
	m_AcceptThread = false;
}

bool UpdateServer::add_client(SOCKET s)
{
	EnterCriticalSection(&m_cs);
	if (nConnNum < MAX_CONNECT)
	{
		for (int i = 0; i < MAX_CONNECT; ++i)
		{
			if (g_queue[i].s == INVALID_SOCKET)
			{
				g_queue[i].s = s;
				g_queue[i].start();
				break;
			}
		}
		++nConnNum;
		LeaveCriticalSection(&m_cs);
		return true;
	}
	else
	{
		LeaveCriticalSection(&m_cs);
		return false;
	}
}

UINT WINAPI UpdateServer::RecvDataThread(void *param)
{
	OutputDebugStringA("===> RecvDataThread BEGIN\n");
	UpdateServer *pThis = (UpdateServer*)param;
	pThis->ThisRecvDataProc();
	OutputDebugStringA("===> RecvDataThread END\n");
	return 0xDEAD888;
}

int UpdateServer::ThisRecvDataProc()
{
	fd_set fd;
	const timeval tv = {2, 0};
	while (!m_bExit)
	{
		FD_ZERO(&fd);
		FD_SET(m_Socket, &fd);
		EnterCriticalSection(&m_cs);
		for (int i = 0; i < MAX_CONNECT; ++i)
		{
			if (INVALID_SOCKET != g_queue[i].s)
			{
				FD_SET(g_queue[i].s, &fd);
			}
		}
		LeaveCriticalSection(&m_cs);
		// 调用select模式进行监听
		int nRes = select(0, &fd, NULL, NULL, &tv);
		if (0 == nRes)// 超时，继续
		{
			Sleep(SLEEP_TIME);
			continue;
		}
		else if (nRes < 0)
		{
			break;
		}
		EnterCriticalSection(&m_cs);
		for (int i = 0; i < MAX_CONNECT; ++i)
		{
			if(INVALID_SOCKET == g_queue[i].s)
				continue;
			SOCKET &cur = g_queue[i].s;
			if (FD_ISSET(cur, &fd))
			{
				char recvBuff[SIZE_1 + 4];
				nRes = recv(cur, recvBuff, SIZE_1, 0);
				if (nRes <= 0)
				{
					FD_CLR(cur,&fd);
					g_queue[i].exit();
					--nConnNum;
				}
				else
				{
					recvBuff[nRes] = 0;
					g_queue[i].callback(recvBuff, nRes);
				}
			}
		}
		LeaveCriticalSection(&m_cs);
		Sleep(SLEEP_TIME);
	}
	m_RecvDataThread = false;
	return 0;
}

void SocketInfo::callback(const char *data, int len)
{
	char buf[SIZE_1 + 4] = {0}, *arg = NULL;
	memcpy(buf, data, len);
	parse_cmd(buf, arg);
	if (0 == strcmp(buf, "size"))
	{
		char szLog[300];
		sprintf_s(szLog, "===> 准备向客户端传输文件：%s\n", arg);
		OutputDebugStringA(szLog);
		strcpy_s(file, g_f.get(arg));
		step[STEP_1] = DONE;
	}else if (0 == strcmp(buf, "down"))
	{
		if ('?' == *arg) total = 0; // 客户端是否不下载
		step[STEP_2] = DONE;
	}else
	{
		char szLog[300];
		sprintf_s(szLog, "===> 收到无法解析的指令: %s\n", data);
		OutputDebugStringA(data);
	}
}

#pragma comment(lib, "winmm.lib")

UINT WINAPI SocketInfo::ParseDataThread(void *param)
{
	OutputDebugStringA("===> ParseDataThread BEGIN\n");
	SocketInfo *pThis = (SocketInfo *)param;
	timeBeginPeriod(1);
	while (INVALID_SOCKET != pThis->s)
	{
		pThis->processing();
		Sleep(SLEEP_TIME);
	}
	pThis->flag = false;
	timeEndPeriod(1);
	OutputDebugStringA("===> ParseDataThread END\n");
	return 0xDead999;
}


void SocketInfo::processing()
{
	if (DONE == step[STEP_1])// Step1: 发送文件概要
	{
		++step[STEP_1];
		OutputDebugStringA("===> STEP 1\n");
		char resp[SIZE_1], arg[SIZE_1], MD5[32+4] = {0};
		total = 0;
		if (0 == _access(file, 0))
		{
			FILE* f = fopen(file, "rb");
			if (f)
			{
				fseek(f, 0, SEEK_END);
				total = ftell(f);
				fclose(f);
			}
			MDFile(file, MD5);
		}
		sprintf_s(arg, "%d,%s", total, MD5);
		make_cmd(resp, "size", arg);
#ifdef _DEBUG
		OutputDebugStringA("===> Send: ");
		OutputDebugStringA(resp);
		OutputDebugStringA("\r\n");
#endif
		::send(s, resp, SIZE_1, 0);
	}else if (DONE == step[STEP_2])// Step2: 传输文件
	{
		++step[STEP_2];
		OutputDebugStringA("===> STEP 2\n");
		int size = total;
		if (size <= 0)
			return;
		FILE *fid = fopen(file, "rb");
		if (fid)
		{
			const int SZ = 32 * 1024; // 32K
			if (NULL == buf) buf = new char[SZ];
			do 
			{
				int once = min(SZ, size), len = once;// 本次传送量
				const char *p = buf;
				fread(buf, once, 1, fid);
				do 
				{
					int sd = ::send(s, p, len, 0);
					if (sd < 0)
						break;
					p += sd;
					len -= sd;
				} while (len > 0);
				if (len > 0)
					break;
				size -= once;
				Sleep(5); // 发送太快客户端可能收不赢
			} while (size > 0);
			fclose(fid);
		}
	}
}
