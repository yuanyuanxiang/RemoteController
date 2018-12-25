#pragma once

#include "../stdafx.h"
#include <assert.h>

#ifdef MAX_LISTEN
#undef MAX_LISTEN
#endif

#ifdef _DEBUG
/// 最大监听客户端数
#define MAX_LISTEN 64
#else
#define MAX_LISTEN 256
#endif

/// 默认缓冲区大小
#define DEFAULT_BUFFER 256

// 错误码对应的错误信息
extern const char *pErrorCode[];

enum 
{
	SocketType_Server = 0, 
	SocketType_Client, 
};

/************************************************************************/
/* socket 错误码定义                                                    */
/************************************************************************/
enum 
{
	_NO__ERROR = 0,		/// no error
	ERROR_WSASTARTUP,	/// WSAStartup() error
	ERROR_SOCKET,		/// socket() error
	ERROR_CONNECT,		/// connect() error
	ERROR_BIND,			/// bind() error
	ERROR_LISTEN,		/// listen() error
	ERROR_IOCTLSOCKET,	/// ioctlsocket() error
	ERROR_SELECT,		/// select() error
	ERROR_ACCEPT,		/// accept() error
	ERROR_CLIENTFULL,	/// 连接已满
	ERROR_TYPEERROR,	/// init() type error
	ERROR_PORTERROR,	/// port已被占用
	ERROR_MAX
};

/**
* @class CSocketBase
* @brief Socket基类，实现基本的收发数据功能，参见BcecrSocket.
*/
class CSocketBase
{
public:
	CSocketBase(void);
	~CSocketBase(void);

	/// 初始化socket[虚函数]
	virtual int init(const char *pIp, int nPort, int nType); //0:server, 1:client
	/// socket退出时进行清理工作[虚函数]
	virtual void unInit();
	/// 接收数据
	int recvData(char *pBuf, int nReadLen, int nTimeOut = 1000); //nTimeOut单位毫秒
	/// 发送数据
	int sendData(const char *pData, int nSendLen);
	/// 通过错误码获取错误信息
	static const char* GetErrorMsg(int nRet) { assert(nRet >= 0 && nRet < ERROR_MAX); return pErrorCode[nRet]; }

protected:
	int m_nType;			/**< socket类型，0:server, 1:client */

	SOCKET m_Socket;		/**< 作为客户端连接的socket */
	SOCKET m_SocketListen;	/**< 作为服务器连接的socket */
	fd_set fdRead;			/**< 服务器端接收数据 */

public:
	char m_chToIp[32];				/**< 对方的IP */
	int  m_nToport;					/**< 对方的端口 */
	char m_chLocalIp[32];			/**< 本地IP */
	int  m_nLocalPort;				/**< 本地端口 */
};
