
// stdafx.cpp : 只包括标准包含文件的源文件
// RemoteController.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"
#include "RemoteControllerDlg.h"

extern CRemoteControllerDlg *g_MainDlg;

void LOCK()
{
	if (g_MainDlg)
		g_MainDlg->g_Lock();
}

void UNLOCK()
{
	if (g_MainDlg)
		g_MainDlg->g_Unlock();
}
