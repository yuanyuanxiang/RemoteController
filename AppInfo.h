
#pragma once

// 表格的列
enum _Column
{
	_id = 0,
	_no,		// 端口
	_ip,		// IP
	_name,		// 名称
	_cpu,		// CPU占用
	_mem,		// 内存占用
	_threads,	// 线程数
	_handles,	// 句柄数
	_runlog,	// 运行时长
	_runtime,	// 启动次数
	_create_time, // 创建日期
	_mod_time,	// 修改日期
	_version,	// 版本
	_cmd_line,	// 命令行
	COLUMNS, 
};

// App 信息
class AppInfo
{
public:
	char ip[64];		// IP
	char name[64];		// 名称
	char cpu[64];		// CPU
	char mem[64];		// 内存
	char threads[64];	// 线程数
	char handles[64];	// 句柄数
	char run_log[64];	// 运行时长
	char run_times[64];	// 重启次数
	char create_time[64];// 创建时间
	char mod_time[64];	// 修改日期
	char version[64];	// 版本
	char cmd_line[_MAX_PATH]; // 命令行

public:
	AppInfo()
	{
		memset(this, 0, sizeof(*this));
	}

	AppInfo(const char *__ip, const char *__name)
	{
		memset(this, 0, sizeof(*this));
		strcpy_s(ip, __ip);
		strcpy_s(name, __name);
	}
};
