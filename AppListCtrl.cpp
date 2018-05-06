// AppListCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteController.h"
#include "AppListCtrl.h"
#include "socket\SocketServer.h"

extern CSocketServer *g_pSocket;

extern const int g_Width[COLUMNS];

// 上一次排序的列
int last_col = -1;

// 本次排序的列
int sort_col = 0;

// 排序方式
bool method = true;

// CAppListCtrl

IMPLEMENT_DYNAMIC(CAppListCtrl, CListCtrl)

// App List Control
CAppListCtrl *g_pList = NULL;

CAppListCtrl::CAppListCtrl()
{
	m_nIndex = -1;
	g_pList = this;
}


CAppListCtrl::~CAppListCtrl()
{
}


void CAppListCtrl::AddColumns(const CString its[], int cols)
{
	for (int i = 0; i < cols; ++i)
	{
		InsertColumn(i, its[i], LVCFMT_CENTER, g_Width[i]);
	}
}


BEGIN_MESSAGE_MAP(CAppListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_RCLICK, &CAppListCtrl::OnNMRClick)
	ON_COMMAND(ID_OP_RESTART, &CAppListCtrl::RestartApp)
	ON_COMMAND(ID_OP_QUERY, &CAppListCtrl::QueryAppInfo)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, &CAppListCtrl::OnLvnColumnclick)
	ON_COMMAND(ID_OP_STOP, &CAppListCtrl::StopApp)
	ON_COMMAND(ID_OP_START, &CAppListCtrl::StartApp)
	ON_COMMAND(ID_OP_UPDATE, &CAppListCtrl::UpdateApp)
END_MESSAGE_MAP()


// CAppListCtrl 消息处理程序


void CAppListCtrl::OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1)
	{
		DWORD dwPos = GetMessagePos();
		CPoint point(LOWORD(dwPos), HIWORD(dwPos));
		CMenu menu;
		// 添加线程操作
		VERIFY(menu.LoadMenu(IDR_LIST_MENU));
		CMenu* popup = menu.GetSubMenu(0);
		ASSERT(popup != NULL);
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
		Lock();
		// 下面的两行代码主要是为了后面的操作为准备的
		// 获取列表视图控件中第一个被选择项的位置
		POSITION m_pstion = GetFirstSelectedItemPosition();
		//该函数获取由pos指定的列表项的索引，然后将pos设置为下一个位置的POSITION值
		m_nIndex = GetNextSelectedItem(m_pstion);
		Unlock();
	}
}


void CAppListCtrl::UpdateAppItem(const char* port, const AppInfo &it)
{
	USES_CONVERSION;
	Lock();
	int n = GetItemCount();
	for(int row = 0; row < n; ++row)
	{
		CString no = GetItemText(row, _no);
		if (0 == strcmp(port, W2A(no)))
		{
			SetItemText(row, _ip, A2W(it.ip));
			SetItemText(row, _name, A2W(it.name));
			SetItemText(row, _cpu, A2W(it.cpu));
			SetItemText(row, _mem, A2W(it.mem));
			SetItemText(row, _threads, A2W(it.threads));
			SetItemText(row, _handles, A2W(it.handles));
			SetItemText(row, _runlog, A2W(it.run_log));
			SetItemText(row, _runtime, A2W(it.run_times));
			SetItemText(row, _create_time, A2W(it.create_time));
			SetItemText(row, _mod_time, A2W(it.mod_time));
			SetItemText(row, _version, A2W(it.version));
			SetItemText(row, _keep_ver, A2W(it.keep_ver));
			SetItemText(row, _cmd_line, A2W(it.cmd_line));
			break;
		}
	}
	Unlock();
}


void CAppListCtrl::AvgColumnWidth(int cols)
{
	Lock();
	for (int i = 0; i < cols; ++i)
	{
		SetColumnWidth(i, g_Width[i]);
	}
	Unlock();
}


void CAppListCtrl::Clear()
{
	Lock();
	int n = 0;
	while (n = GetItemCount())
	{
		DeleteItem(n-1);
	}
	Unlock();
}


void CAppListCtrl::InsertAppItem(const char* port)
{
	USES_CONVERSION;
	Lock();
	int n = GetItemCount();
	CString s;
	s.Format(_T("%d"), n+1);
	InsertItem(n, s);
	SetItemText(n, _no, A2W(port));
	Unlock();
}


void CAppListCtrl::DeleteAppItem(const char* port)
{
	USES_CONVERSION;
	Lock();
	int n = GetItemCount();
	for(int row = 0; row < n; ++row)
	{
		CString no = GetItemText(row, _no);
		if (0 == strcmp(port, W2A(no)))
		{
			DeleteItem(row);
			// 后面需要重排序
			--n;
			for(int i = row; i < n; ++i)
			{
				CString s;
				s.Format(_T("%d"), i + 1);
				SetItemText(i, _id, s);
			}
			break;
		}
	}
	Unlock();
}


void CAppListCtrl::RestartApp()
{
	if (-1 != m_nIndex && IDYES == MessageBox(_T("确定\"重启\"此程序吗?"), _T("警告"), MB_ICONWARNING | MB_YESNO))
	{
		TRACE("======> RestartApp index = %d\n", m_nIndex);
		USES_CONVERSION;
		Lock();
		CString no = GetItemText(m_nIndex, _no);
		Unlock();
		g_pSocket->SendCommand(RESTART, W2A(no));
	}
}


void CAppListCtrl::QueryAppInfo()
{
	if (-1 != m_nIndex)
	{
		TRACE("======> QueryAppInfo index = %d\n", m_nIndex);
		USES_CONVERSION;
		Lock();
		CString no = GetItemText(m_nIndex, _no);
		Unlock();
		g_pSocket->SendCommand(REFRESH, W2A(no));
	}
}


// 排序回调
int CALLBACK comp(LPARAM p1, LPARAM p2, LPARAM s)
{
	int row1 = (int) p1;
	int row2 = (int) p2;
	const CAppListCtrl *lc = (CAppListCtrl*)s;
	CString lps1 = lc->GetItemText(row1, sort_col);
	CString lps2 = lc->GetItemText(row2, sort_col);
	if (sort_col < _ip || (_name < sort_col && sort_col < _create_time))
	{
		USES_CONVERSION;
		double d1 = atof(W2A(lps1)), d2 = atof(W2A(lps2));
		return method ? d1 - d2 > 0 : d2 - d1 > 0;
	}
	else
	{
		return method ? lps1.CompareNoCase(lps2) : lps2.CompareNoCase(lps1);
	}
}


void CAppListCtrl::OnLvnColumnclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	
	Lock();
	int n = GetItemCount();
	if (n)
	{
		sort_col = pNMLV->iSubItem;
		method = (last_col==sort_col ? !method : true);
		for (int i = 0; i < n; ++i)
			SetItemData(i, i);
		SortItems(&comp, (DWORD_PTR)this);
		last_col = sort_col;
	}
	Unlock();

	*pResult = 0;
}


void CAppListCtrl::StopApp()
{
	if (-1 != m_nIndex && IDYES == MessageBox(_T("确定\"停止\"此程序吗?"), _T("警告"), MB_ICONWARNING | MB_YESNO))
	{
		TRACE("======> StopApp index = %d\n", m_nIndex);
		USES_CONVERSION;
		Lock();
		CString no = GetItemText(m_nIndex, _no);
		Unlock();
		g_pSocket->SendCommand(STOP, W2A(no));
	}
}


void CAppListCtrl::StartApp()
{
	if (-1 != m_nIndex)
	{
		TRACE("======> StartApp index = %d\n", m_nIndex);
		USES_CONVERSION;
		Lock();
		CString no = GetItemText(m_nIndex, _no);
		Unlock();
		g_pSocket->SendCommand(START, W2A(no));
	}
}


void CAppListCtrl::UpdateApp()
{
	if (-1 != m_nIndex && IDYES == MessageBox(_T("确定\"升级\"此程序吗?"), _T("警告"), MB_ICONWARNING | MB_YESNO))
	{
		TRACE("======> UpdateApp index = %d\n", m_nIndex);
		USES_CONVERSION;
		Lock();
		CString no = GetItemText(m_nIndex, _no);
		Unlock();
		std::string cmd = MAKE_CMD(UPDATE, "a");
		g_pSocket->SendCommand(cmd.c_str(), W2A(no));
	}
}
