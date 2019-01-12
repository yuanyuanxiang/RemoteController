#pragma once
#pragma once
#include <WinSock2.h>
#include <process.h>

#ifdef _DEBUG
#define MAX_CONNECT 100
#else
#define MAX_CONNECT 256
#endif

// 请求指令的固定长度
#define SIZE_1 100

// req					| resp
// (1) size:file		| size:bytes,md5
// (2) down:file		| ......

// 拼接请求指令
#define make_cmd(buf, cmd, arg) {memset(buf,0,SIZE_1); sprintf_s(buf,"%s:%s",cmd,arg);}

// 解析指令
#define parse_cmd(buf, arg) {arg=buf; while(*arg && ':'!=*arg)++arg; if(*arg)*arg++=0;}

// 目录访问器[成员易变的]
typedef struct folder
{
	char buf[_MAX_PATH], *pos;
	folder()
	{
		// 获取程序当前目录
		GetModuleFileNameA(NULL, buf, sizeof(buf));
		pos = buf;
		while ('\0' != *pos) ++pos;
		while ('\\' != *pos) --pos;
		++pos;
	}
	// 获取当前目录下的文件
	inline const char* get(const char *file)
	{
		strcpy(pos, file);
		return buf;
	}
}folder;

struct SocketInfo
{
	enum { STEP_1 = 0, STEP_2, STEP_NUM };
	enum { NOT_DONE = 0, DONE };
	SOCKET s;					// socket
	int total;					// 文件总字节
	bool flag;					// 线程状态
	int step[STEP_NUM];				// 通知
	char *buf;					// 读文件缓存
	char file[_MAX_PATH];		// 文件全路径
	SocketInfo() : s(INVALID_SOCKET), total(0), flag(false), buf(NULL)
	{
		step[STEP_1] = step[STEP_2] = NOT_DONE;
	}
	~SocketInfo() { if (buf) delete [] buf; }
	void callback(const char *data, int len); // 收到数据时回调
	void processing();
	void exit() {
		closesocket(s);
		s = INVALID_SOCKET;
		while(flag)
			Sleep(10);
		step[STEP_1] = step[STEP_2] = NOT_DONE;
		total = 0;
	}
	void start(){
		HANDLE h = NULL;
		flag = h = (HANDLE)_beginthreadex(0, 0, ParseDataThread, this, 0, 0);
		CloseHandle(h);
	}
	static UINT WINAPI ParseDataThread(void *param);
};

/************************************************************************
* @class UpdateServer
* @brief 被守护程序升级服务端
* @details 升级分为2步：
*	 req					| resp
*	 (1) size:file			| size:bytes,md5
*	 (2) down:file			| ......
第（1）步，客户端发送要升级的文件名，服务端进行校验，返回文件字节数和MD5码
第（2）步：客户端校验MD5码，发送升级指令，服务端解析该指令并开始文件传输
/************************************************************************/
class UpdateServer
{
private:
	char m_strIp[64];
	int m_nPort;
	SOCKET m_Socket;
	bool m_bExit;					// 程序运行状态
	bool m_AcceptThread;			// 线程状态
	bool m_RecvDataThread;			// 线程状态
	int nConnNum;					// 连接数
	SocketInfo g_queue[MAX_CONNECT];	/**< server端处理所有的待决连接 */
	CRITICAL_SECTION m_cs;

	void CheckIO();

	bool add_client(SOCKET s);

	int ThisRecvDataProc();

public:
	UpdateServer(void);
	~UpdateServer(void);

	// 初始化socket
	int init(const char *pIp, int nPort);

	// socket退出时进行清理工作
	void unInit();

	static UINT WINAPI AcceptThread(void *param);

	static UINT WINAPI RecvDataThread(void *param);
};
