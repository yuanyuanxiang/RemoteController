
// RemoteControllerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "RemoteControllerDlg.h"
#include "afxdialogex.h"
#include "IPConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 表格的列名
const CString items[COLUMNS] = { 
	_T("序号"), 
	_T("端口"), 
	_T("IP"), 
	_T("名称"), 
	_T("CPU"), 
	_T("内存"), 
	_T("线程"),
	_T("句柄"),
	_T("运行时长"), 
	_T("启动次数"), 
	_T("创建日期"),
	_T("修改日期"),
	_T("版本"), 
	_T("守护程序"), 
	_T("位置")
};

// 每列宽度
const int g_Width[COLUMNS] = {
	55, // 序号
	80, // 端口
	130, // IP
	200, // 名称
	80, // CPU
	100, // 内存
	80, // 线程
	80, // 句柄
	100, // 运行时长
	80, // 启动次数
	155, // 创建日期
	155, // 修改日期
	80, // 版本
	80, // 守护程序版本
	600,// 位置
};

// Socket服务端
CSocketServer *g_pSocket = NULL;

CRemoteControllerDlg *g_MainDlg = NULL;

const char* GetLocalHost()
{
	static char localhost[128] = { "127.0.0.1" };
	char hostname[128] = { 0 };
	if (0 == gethostname(hostname, 128))
	{
		hostent *host = gethostbyname(hostname);
		// 将ip转换为字符串
		char *hostip = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
		memcpy(localhost, hostip, strlen(hostip));
	}
	return localhost;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteControllerDlg 对话框


CRemoteControllerDlg::CRemoteControllerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRemoteControllerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	strcpy_s(m_strIp, "127.0.0.1");
	m_nPort = 9999;

	m_pServer = NULL;
	m_bAdvanced = false;
	InitializeCriticalSection(&m_cs);
}


CRemoteControllerDlg::~CRemoteControllerDlg()
{
	WSACleanup();
	DeleteCriticalSection(&m_cs);
}


void CRemoteControllerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_APPLICATION_LIST, m_ListApps);
}


BEGIN_MESSAGE_MAP(CRemoteControllerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_QUIT_APP, &CRemoteControllerDlg::OnQuitApp)
	ON_COMMAND(ID_REFRESH_ALL, &CRemoteControllerDlg::OnRefreshAll)
	ON_COMMAND(ID_POWEROFF_ALL, &CRemoteControllerDlg::OnPoweroffAll)
	ON_COMMAND(ID_IPCONFIG, &CRemoteControllerDlg::OnIpconfig)
	ON_COMMAND(ID_APP_ABOUT, &CRemoteControllerDlg::OnAppAbout)
	ON_WM_INITMENUPOPUP()
	ON_UPDATE_COMMAND_UI(ID_POWEROFF_ALL, &CRemoteControllerDlg::OnUpdatePoweroffAll)
	ON_COMMAND(ID_REBOOT_SYSTEM, &CRemoteControllerDlg::OnRebootSystem)
	ON_UPDATE_COMMAND_UI(ID_REBOOT_SYSTEM, &CRemoteControllerDlg::OnUpdateRebootSystem)
	ON_UPDATE_COMMAND_UI(ID_REFRESH_ALL, &CRemoteControllerDlg::OnUpdateRefreshAll)
	ON_COMMAND(ID_UPDATE, &CRemoteControllerDlg::OnUpdate)
	ON_COMMAND(ID_SETTIME, &CRemoteControllerDlg::OnSettime)
	ON_COMMAND(ID_STOPALL, &CRemoteControllerDlg::OnStopall)
	ON_UPDATE_COMMAND_UI(ID_STOPALL, &CRemoteControllerDlg::OnUpdateStopall)
	ON_COMMAND(ID_STARTALL, &CRemoteControllerDlg::OnStartall)
	ON_UPDATE_COMMAND_UI(ID_STARTALL, &CRemoteControllerDlg::OnUpdateStartall)
END_MESSAGE_MAP()


// CRemoteControllerDlg 消息处理程序

BOOL CRemoteControllerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_SHOW);

	// 为列表视图控件添加全行选中和栅格风格
	m_ListApps.SetExtendedStyle(m_ListApps.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	// 为列表视图控件添加列
	m_ListApps.AddColumns(items, COLUMNS);

	// socket
	WSADATA wsaData; // Socket
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	m_pServer = new CSocketServer;
	// settings.ini
	GetModuleFileNameA(NULL, m_strConf, _MAX_PATH);
	char *p = m_strConf;
	while('\0' != *p) ++p;
	while('\\' != *p) --p;
	strcpy(p, "\\settings.ini");
	GetPrivateProfileStringA("settings", "localIp", "", m_strIp, 64, m_strConf);
	if (m_strIp[0] == '\0')
		strcpy_s(m_strIp, GetLocalHost());

	m_nPort = GetPrivateProfileIntA("settings", "port", 9999, m_strConf);
	TRACE("======> Start socket: Ip = %s, Port = %d\n", m_strIp, m_nPort);
	m_pServer->init(m_strIp, m_nPort, SocketType_Server);
	g_pSocket = m_pServer;

	m_bAdvanced = GetPrivateProfileIntA("settings", "advanced", 0, m_strConf);
	CRemoteControllerDlg *g_MainDlg = this;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CRemoteControllerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteControllerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteControllerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CRemoteControllerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (NULL != m_pServer)
	{
		m_pServer->unInit();
		delete m_pServer;
		m_pServer = NULL;
	}
}


void CRemoteControllerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_ListApps.GetSafeHwnd())
	{
		CRect rt;
		GetClientRect(&rt);
		const int EDGE = 16;
		m_ListApps.MoveWindow(CRect(rt.left + EDGE, rt.top + EDGE, 
			rt.right - EDGE, rt.bottom - EDGE), TRUE);
		m_ListApps.AvgColumnWidth(COLUMNS);
	}
}


void CRemoteControllerDlg::OnQuitApp()
{
	SendMessage(WM_CLOSE, 0, 0);
}


BOOL CRemoteControllerDlg::PreTranslateMessage(MSG* pMsg)
{
	// 屏蔽 ESC/Enter 关闭窗体
	if( pMsg->message == WM_KEYDOWN && 
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN) )
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CRemoteControllerDlg::OnRefreshAll()
{
	g_pSocket->SendCommand(REFRESH);
}


void CRemoteControllerDlg::OnPoweroffAll()
{
	int i = 0, nRet = 0;
	do 
	{
		nRet = MessageBox(_T("点击\"确定\"将试图关闭所有连接到本控制器的设备!")\
			_T("\r\n请不要轻易点击\"确定\"按钮, 盼三思而后行!"), 
			_T("警告"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
	} while (IDYES == nRet && ++i < 3);
	if (3 == i)
		g_pSocket->ControlDevice(SHUTDOWN);
}


void CRemoteControllerDlg::OnIpconfig()
{
	CIPConfigDlg dlg;
	dlg.m_strIpAddr = m_strIp;
	dlg.m_nPort = m_nPort;
	if (IDOK == dlg.DoModal())
	{
		if (m_nPort != dlg.m_nPort || CString(m_strIp) != dlg.m_strIpAddr)
		{
			USES_CONVERSION;
			strcpy_s(m_strIp, W2A(dlg.m_strIpAddr));
			m_nPort = dlg.m_nPort;
			WritePrivateProfileStringA("settings", "localIp", m_strIp, m_strConf);
			char buf[64];
			sprintf_s(buf, "%d", m_nPort);
			WritePrivateProfileStringA("settings", "port", buf, m_strConf);
			m_pServer->unInit();
			m_pServer->init(m_strIp, m_nPort, SocketType_Server);
			m_ListApps.Clear();
		}
	}
}


void CRemoteControllerDlg::OnAppAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}


void CRemoteControllerDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	CDialogEx::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);

	ASSERT(pPopupMenu != NULL);
	// Check the enabled state of various menu items.

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);

	// Determine if menu is popup in top-level menu and set m_pOther to
	// it if so (m_pParentMenu == NULL indicates that it is secondary popup).
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu;    // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
		// Child windows don't have menus--need to go to the top!
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					// When popup is found, m_pParentMenu is containing menu.
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}

	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
		state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.

		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;       // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE);   // Popups are never auto disabled.
		}
		else
		{
			// Normal menu item.
			// Auto enable/disable if frame window has m_bAutoMenuEnable
			// set and command is _not_ a system command.
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}

		// Adjust for menu deletions and additions.
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
}


void CRemoteControllerDlg::OnUpdatePoweroffAll(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bAdvanced && m_ListApps.GetItemCount());
}


void CRemoteControllerDlg::OnRebootSystem()
{
	int i = 0, nRet = 0;
	do 
	{
		nRet = MessageBox(_T("点击\"确定\"将试图重启所有连接到本控制器的设备!")\
			_T("\r\n请不要轻易点击\"确定\"按钮, 盼三思而后行!"), 
			_T("警告"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
	} while (IDYES == nRet && ++i < 3);
	if (3 == i)
		g_pSocket->ControlDevice(REBOOT);
}


void CRemoteControllerDlg::OnUpdateRebootSystem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bAdvanced && m_ListApps.GetItemCount());
}


void CRemoteControllerDlg::OnUpdateRefreshAll(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_ListApps.GetItemCount());
}


void CRemoteControllerDlg::OnUpdate()
{
	if (IDYES == MessageBox(_T("确定\"升级\"守护程序吗?"), _T("警告"), MB_ICONWARNING | MB_YESNO))
	{
		g_pSocket->SendCommand(UPDATE);
	}
}


void CRemoteControllerDlg::OnSettime()
{
	if (IDYES == MessageBox(_T("确定\"同步\"所有时间吗?"), _T("警告"), MB_ICONWARNING | MB_YESNO))
	{
		SYSTEMTIME st;
		GetLocalTime(&st);
		char buf[64];
		sprintf_s(buf, "settime:%d,%d,%d,%d,%d,%d,%d,%d", st.wYear, st.wMonth, st.wDayOfWeek, 
			st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		g_pSocket->ControlDevice(buf);
	}
}


void CRemoteControllerDlg::OnStopall()
{
	int i = 0, nRet = 0;
	do 
	{
		nRet = MessageBox(_T("点击\"确定\"将停止所有程序! 盼三思而后行!"), 
			_T("警告"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
	} while (IDYES == nRet && ++i < 3);
	if (3 == i)
		g_pSocket->SendCommand(STOP);
}


void CRemoteControllerDlg::OnUpdateStopall(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bAdvanced && m_ListApps.GetItemCount());
}


void CRemoteControllerDlg::OnStartall()
{
	if(IDYES == MessageBox(_T("点击\"确定\"将启动所有程序!"), 
			_T("提示"), MB_ICONINFORMATION | MB_YESNO | MB_DEFBUTTON2))
		g_pSocket->SendCommand(START);
}


void CRemoteControllerDlg::OnUpdateStartall(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bAdvanced && m_ListApps.GetItemCount());
}
