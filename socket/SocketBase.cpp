#include "SocketBase.h"
#include <process.h>


// 错误码对应的错误信息
const char *pErrorCode[] =
{
	"no error",
	"WSAStartup() error",
	"socket() error",
	"connect() error",
	"bind() error",
	"listen() error",
	"ioctlsocket() error",
	"select() error",
	"accept() error",
	"连接已满",
	"init() type error", 
};

CSocketBase::CSocketBase(void)
{
	m_nType = 0;
	m_nToport = 0;
	m_nLocalPort = 0;

	m_Socket = INVALID_SOCKET;
	m_SocketListen = INVALID_SOCKET;

	memset(m_chLocalIp, 0, sizeof(m_chLocalIp));
	memset(m_chToIp, 0, sizeof(m_chToIp));
}


CSocketBase::~CSocketBase(void)
{
}


/** 
* @brief 初始化一个 socket
* @param[in] *pIp	服务端IP
* @param[in] nPort	通信端口
* @param[in] nType	socket类型，0：server 1:client
* @retval	 int
* @return	 错误码,可通过GetErrorMsg()获取错误信息  
*/
int CSocketBase::init(const char *pIp, int nPort, int nType)
{
	int nRet = _NO__ERROR;

	/// 创建socket，并建立连接
	do 
	{
		/// 初始化一个服务端socket
		if (SocketType_Server == nType)
		{		
			/////////////////////////////////////////////////////////////////////////////////
			// 设置套接口的选项
			// 在send()的时候，返回的是实际发送出去的字节(同步)或发送到socket缓冲区的字节
			// (异步);系统默认的状态发送和接收一次为8688字节(约为8.5K)；在实际的过程中发送数据
			// 和接收数据量比较大，可以设置socket缓冲区，而避免了send(),recv()不断的循环收发
			////////////////////////////////////////////////////////////////////////////////

			/// 发送缓冲区
			int nSendBuf = 1024 * 1024 * 2;
			::setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuf, sizeof(int));
			/// 接收缓冲区
			int nRecvBuf = 1024 * 1024 * 2;
			::setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuf, sizeof(int));
			bool bConditionalAccept = true;
			::setsockopt(m_Socket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (const char *)&bConditionalAccept, sizeof(bool));

			/// 创建服务端socket
			sockaddr_in addrServer;
			m_SocketListen = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
			if (m_SocketListen == INVALID_SOCKET)
			{
				nRet = ERROR_SOCKET;
				break;
			}
			/// 设定服务器IP 和 监听PORT
			addrServer.sin_family = AF_INET;
			addrServer.sin_addr.S_un.S_addr = inet_addr(pIp);
			addrServer.sin_port = htons(nPort);
			/// 绑定SOCKET 和 PORT
			if (SOCKET_ERROR == bind(m_SocketListen,(const sockaddr*)&addrServer, sizeof(addrServer)) )
			{
				nRet = ERROR_BIND;
				break;
			}
			/// 监听端口
			if (SOCKET_ERROR == listen(m_SocketListen, MAX_LISTEN))
			{
				nRet = ERROR_LISTEN;
				break;
			}
			/// 非阻塞模式设定
			ULONG ul = 1;
			if (SOCKET_ERROR == ioctlsocket(m_SocketListen, FIONBIO, &ul))
			{
				nRet = ERROR_IOCTLSOCKET;
				break;
			}
		}
		/// 初始化一个客户端socket
		else if (SocketType_Client == nType)
		{
			m_Socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
			if (INVALID_SOCKET == m_Socket)
			{
				nRet = ERROR_SOCKET;
				break;
			}
			/// 发送缓冲区
			int nSendBuf = 1024 * 1024 * 2;
			::setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&nSendBuf, sizeof(int));
			/// 接收缓冲区
			int nRecvBuf = 1024 * 1024 * 2;
			::setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&nRecvBuf, sizeof(int));
			bool bConditionalAccept = true;
			::setsockopt(m_Socket, SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (const char *)&bConditionalAccept, sizeof(bool));

			m_nToport = nPort;
			memcpy(m_chToIp, pIp, strlen(pIp));

			sockaddr_in addrAdpter;
			memset(&addrAdpter, 0, sizeof(addrAdpter));
			addrAdpter.sin_family = AF_INET;
			addrAdpter.sin_port = htons(nPort);
			addrAdpter.sin_addr.s_addr = inet_addr(pIp);

			/// 和服务端建立连接
			nRet = ::connect(m_Socket, (sockaddr *)&addrAdpter, sizeof(addrAdpter));
			if (SOCKET_ERROR == nRet)
			{
				::closesocket(m_Socket);
				m_Socket = INVALID_SOCKET;

				nRet = ERROR_CONNECT;
				break;
			}

			//socket转换为非阻塞模式,当与某个视频服务器传输中断时,防止传输阻塞
			ULONG ul = 1;   
			if (SOCKET_ERROR == ioctlsocket(m_Socket, FIONBIO, &ul))
			{
				nRet = ERROR_IOCTLSOCKET;
				break;
			}
		}
		else
			nRet = ERROR_TYPEERROR;
	} while (false);

	return nRet;
}


void CSocketBase::unInit()
{
	if (INVALID_SOCKET != m_Socket)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

	if (INVALID_SOCKET != m_SocketListen)
	{
		closesocket(m_SocketListen);
		m_SocketListen = INVALID_SOCKET;
		OutputDebugStringA("\n======> CSocketBase Server Socket Closed.\n\n");
	}
}


/**
* @brief 收取数据
* @param[in] *pBuf			缓存区
* @param[in] nReadLen		数据长度
* @param[in] nTimeOut		超时时间
* @return 返回操作代码(大于等于0为成功)
* @retval int
*/
int CSocketBase::recvData(char *pBuf, int nReadLen, int nTimeOut)
{
	if ((INVALID_SOCKET == m_Socket) || (NULL == pBuf) || (0 == nReadLen))
		return -1;

	int ret = 0;
	struct timeval time;
	time.tv_sec = nTimeOut/1000;
	time.tv_usec = (nTimeOut%1000) * 1000;

	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(m_Socket, &fd);

	pBuf[0] = '\0';
	ret = ::select(m_Socket+1, &fd, NULL, NULL, &time);
	if ( ret )
	{
		if ( FD_ISSET(m_Socket, &fd) )
		{
			ret = ::recv(m_Socket, pBuf, nReadLen, 0);
			ret = (ret <= 0) ? -1 : ret;
		}
	}
	else if(ret < 0)
	{
		ret = -1;
	}

	return ret;
}

/**
* @brief 发送数据
* @param[in] *pData			缓存区
* @param[in] nSendLen		数据长度
* @return 返回操作代码
* @retval int
*/
int CSocketBase::sendData(const char *pData, int nSendLen)
{
	if ((INVALID_SOCKET == m_Socket) || (NULL == pData) || (0 == nSendLen))
		return -1;

	int nRet = 0;
	int ret = 0;

	struct timeval time;
	time.tv_sec = 2;
	time.tv_usec =0;

	fd_set fdSend;
	int nLen = nSendLen;
	const char *pTmp = pData;

	while (nLen > 0)
	{
		FD_ZERO(&fdSend);
		FD_SET(m_Socket, &fdSend);

		ret = ::select(m_Socket+1, NULL, &fdSend, NULL, &time);
		if ( 1== ret )
		{
			if ( FD_ISSET(m_Socket, &fdSend) )
			{
				ret = ::send(m_Socket, pTmp, nLen, 0);
				if (ret <= 0)
				{
					nRet = -1;
					break;
				}

				nLen -= ret;
				pTmp += ret;

			}
		}
		else if ( ret < 0)
		{
			nRet = ret;
			break;
		}
		else if (0 == ret)
		{
			nRet = 0;
			break;
		}
	}

	return nRet;
}
