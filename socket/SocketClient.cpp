#include "SocketClient.h"
#include <process.h>
#include "..\AppListCtrl.h"
#include "SocketServer.h"
#include "..\RemoteControllerDlg.h"

#ifndef X64
#ifdef _DEBUG
#pragma comment(lib, "Debug/tinyxml.lib")
#else
#pragma comment(lib, "Release/tinyxml.lib")
#endif
#else
#endif

extern CAppListCtrl *g_pList;

#ifndef BUFFER_LENGTH
	#define BUFFER_LENGTH 4096
#endif

// 解析信令的线程
UINT WINAPI CSocketClient::ParseThread(LPVOID param)
{
	CSocketClient *pThis = (CSocketClient*)param;
	pThis->m_bIsParsing = true;
	while (pThis->m_bAlive)
	{
		Sleep(10);
		pThis->ParseData();
	}
	pThis->m_bIsParsing = false;
	return 0xDead00A1;
}


// 收取APP数据的线程
UINT WINAPI CSocketClient::ReceiveThread(LPVOID param)
{
	CSocketClient *pThis = (CSocketClient*)param;
	pThis->m_bIsReceiving = true;
	while (pThis->m_bAlive)
	{
		Sleep(10);
		char buffer[BUFFER_LENGTH];
		int nRet = pThis->recvData(buffer, BUFFER_LENGTH);
		(nRet < 0) ? pThis->Disconnect() : pThis->ReceiveData(buffer, nRet);
	}
	pThis->m_bIsReceiving = false;
	return 0xDead00A2;
}


/**
* @brief 新的Socket客户端
* @param[in] client 通信socket
* @param[in] *Ip 客户端Ip
* @param[in] port 客户端端口
*/
CSocketClient::CSocketClient(SOCKET client, const char *Ip, int port)
{
	m_tick = 0;
	m_nType = 1;
	m_Socket = client;
	strcpy_s(m_chToIp, Ip);
	m_nToport = port;
	m_bExit = false;
	m_bAlive = true;
	m_bIsParsing = false;
	m_bIsReceiving = false;
	m_RingBuffer = new RingBuffer(2 * 1024 * 1024);
	m_RecvBuffer = new char[BUFFER_LENGTH];
	m_xmlParser = NULL;
	m_nAliveTime = ALIVE_TIME;
	m_nSrcPort = port;
	sprintf_s(m_strSrcPort, "%d", port);
	memset(m_strName, 0, 64);
	_beginthreadex(NULL, 0, &ParseThread, this, 0, NULL);
	_beginthreadex(NULL, 0, &ReceiveThread, this, 0, NULL);
	char str[256];
	sprintf_s(str, "发现新的Socket客户端: [%d] %s:%d.\n", m_Socket, m_chToIp, m_nToport);
	OutputDebugStringA(str);
}


CSocketClient::~CSocketClient(void)
{
	delete m_RingBuffer;
	delete [] m_RecvBuffer;
	if (NULL != m_xmlParser)
		delete m_xmlParser;
}


void CSocketClient::unInit()
{
	m_bExit = true;
	m_bAlive = false;
	while (m_bIsParsing || m_bIsReceiving)
		Sleep(10);

	CSocketBase::unInit();
}


void CSocketClient::Disconnect()
{
	char str[256];
	sprintf(str, "Socket客户端: [%d] %s:%d 关闭.\n", m_Socket, m_chToIp, m_nToport);
	g_pList->PostMessage(MSG_DeleteApp, m_nSrcPort);
	OutputDebugStringA(str);
	m_bExit = true;
	m_bAlive = false;
}

// 获取父节点的名为name的子节点的值
inline const char* GetValue(const TiXmlElement* pParent, const char* pName)
{
	const TiXmlElement* pChild = pParent ? pParent->FirstChildElement(pName) : NULL;
	const char *text = pChild ? pChild->GetText() : "";
	return text ? text : "";
}

// SYSTEMTIME转time_t
inline time_t SystemTime2Time_t(const SYSTEMTIME &st)
{
	struct tm tick = { st.wSecond, st.wMinute, st.wHour, 
		st.wDay, st.wMonth - 1, st.wYear - 1900, st.wDayOfWeek, 0, 0 };
	return mktime(&tick);
}

/**
<?xml version="1.0" encoding="GB2312" standalone="yes"?>
<request command="keepAlive">
<parameters>
	<nAliveTime>%d</nAliveTime>
	<szName>%s</szName>
	<szCpu>%s</szCpu>
	<szMem>%d</szMem>
	<szThreads>%s</szThreads>
	<szHandles>%d</szHandles>
	<szRunLog>%s</szRunLog>
	<szRunTimes>%d</szRunTimes>
	<szCreateTime>%s</szCreateTime>
	<szModTime>%s</szModTime>
	<szFileSize>%.2fM</szFileSize>
	<szVersion>%s</szVersion>
	<szKeeperVer>%s</szKeeperVer>
	<szCmdLine>%s</szCmdLine>
	<szDiskFreeSpace>%s</szDiskFreeSpace>
	<szStatus>%s</szStatus>
	<szCurrentTime>%s</szCurrentTime>
</parameters>
</request>
*/
void CSocketClient::ReadSipXmlInfo(const char *buffer, int nLen)
{
	SYSTEMTIME st;// current time
	GetLocalTime(&st);
	time_t tick = SystemTime2Time_t(st);
	char seq[32] = { 0 }, cid[32] = { 0 };
	const char *xml = GetSeqAndCallId(buffer, seq, cid);
	if (0 == *xml)
		return;
	OutputDebugStringA(xml);
	if (NULL == m_xmlParser)
		m_xmlParser = new TiXmlDocument();
	m_xmlParser->Parse(xml, 0, TIXML_DEFAULT_ENCODING);
	TiXmlElement* rootElement = m_xmlParser->RootElement();
	if (NULL == rootElement)
	{
		m_xmlParser->Clear();
		return;
	}
	TiXmlAttribute* attri = rootElement->FirstAttribute();
	const char *cmdType = attri ? attri->Value() : NULL;// command类型
	if (NULL == cmdType)
	{
		m_xmlParser->Clear();
		return;
	}
	TiXmlElement* parameters = rootElement->FirstChildElement("parameters");
	if (NULL == rootElement)
	{
		m_xmlParser->Clear();
		return;
	}
	// 为了兼容以前的版本，保留了大写的"KeepAlive"
	if (0 == strcmp(KEEPALIVE, cmdType) || 0 == strcmp("KeepAlive", cmdType))
	{
		int clr = 0;
		if (m_tick)
		{
			int n = (clock() - m_tick)/2;
			clr = n > 100 ? COLOR_RED : COLOR_DEFAULT;
			m_tick = 0;
			TRACE("======> [%s]时延预估：%dms.\n", GetIp(), n);
		}
		int aliveTime = atoi(GetValue(parameters, "nAliveTime"));
		strcpy_s(item.ip, m_chToIp);
		strcpy_s(item.version, GetValue(parameters, "szVersion"));
		if (0 == item.name[0]){
			strcpy_s(item.name, GetValue(parameters, "szName"));
			std::string ver = g_pSocket->CheckUpdate(item.name);
			if (false == ver.empty() && strcmp(ver.c_str(), item.version) > 0)// 检查并提示有新版本
			{
				char info[128];
				sprintf_s(info, "\"%s\"的当前版本为%s, 服务器已经发布新版本。"\
					"我们将稍后为您更新至版本%s。", item.name, item.version, ver.c_str());
				std::string cmd = MAKE_CMD(NOTICE, info);
				sendData(cmd.c_str(), cmd.length());
				Sleep(10);
			}
		}
		strcpy_s(item.cpu, GetValue(parameters, "szCpu"));
		strcpy_s(item.mem, GetValue(parameters, "szMem"));
		strcpy_s(item.threads, GetValue(parameters, "szThreads"));
		strcpy_s(item.handles, GetValue(parameters, "szHandles"));
		strcpy_s(item.run_log, GetValue(parameters, "szRunLog"));
		strcpy_s(item.run_times, GetValue(parameters, "szRunTimes"));
		strcpy_s(item.create_time, GetValue(parameters, "szCreateTime"));
		strcpy_s(item.mod_time, GetValue(parameters, "szModTime"));
		strcpy_s(item.file_size, GetValue(parameters, "szFileSize"));
		if (0 == strcmp("", item.version)) strcpy_s(item.version, "无");
		strcpy_s(item.keep_ver, GetValue(parameters, "szKeeperVer"));
		strcpy_s(item.cmd_line, GetValue(parameters, "szCmdLine"));
		strcpy_s(item.disk_info, GetValue(parameters, "szDiskFreeSpace"));
		const char *status = GetValue(parameters, "szStatus");
		strcpy_s(item.status, status);
		if (COLOR_DEFAULT == clr)
			clr = strcmp("异常", item.status) ? 
			(strcmp("未检测", item.status) ? COLOR_DEFAULT : COLOR_YELLOW): COLOR_RED;
		if (COLOR_DEFAULT == clr && g_MainDlg->IsDetectTimeError())
		{
			int n[8] = { 0 }, i = 0;
			char sendTime[64];// send msg time
			strcpy_s(sendTime, GetValue(parameters, "szCurrentTime"));
			for(char *p = strtok(sendTime, ","); NULL != p && i < 8; ++i)
			{ 
				n[i] = atoi(p);
				p = strtok(NULL, ",");
			}
			if (sendTime[0])
			{
				SYSTEMTIME st_src = { n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7] };
				time_t tick_src = SystemTime2Time_t(st_src);
				int error = abs(int(tick - tick_src) * 1000 + st.wMilliseconds - st_src.wMilliseconds);
				clr = error > 3000 ? (error > 6000 ? COLOR_BLUE2 : COLOR_BLUE1) : COLOR_DEFAULT;
			}
		}
		g_pList->PostMessage(MSG_ChangeColor, m_nSrcPort, clr);
		g_pList->PostMessage(MSG_UpdateApp, m_nSrcPort, (LPARAM)&item);
		char arg[64] = { 0 };
		m_nAliveTime = max(aliveTime, 1);
		sprintf_s(arg, "%d", m_nAliveTime);
		std::string cmd = MAKE_CMD(KEEPALIVE, arg);
		sendData(cmd.c_str(), cmd.length());
	}
	// 为了兼容以前的版本，保留了大写的"Register"
	// 后续将对注册信令进行验证
	else if(0 == strcmp(REGISTER, cmdType) || 0 == strcmp("Register", cmdType))
	{
		/**
		<?xml version="1.0" encoding="GB2312" standalone="yes"?>
		<request command="register">
		  <parameters>
		    <szAppId>%s</szAppId>
		    <szPassword>%s</szPassword>
		  </parameters>
		</request>
		*/
		// 回复注册成功
		std::string cmd = MAKE_CMD(REGISTER, "success");
		sendData(cmd.c_str(), cmd.length());
	}

	m_xmlParser->Clear();
}
