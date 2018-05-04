#include <string>

// 生成一个控制信令(cmd:arg - 冒号中间没有空格, arg为空表示无参数)
inline std::string MAKE_CMD(const char *cmd, const char *arg)
{
	char buffer[64];
	sprintf_s(buffer, "%s%s%s", cmd, arg ? ":" : "", arg ? arg : "");
	return buffer;
}

// 解析一个控制信令(返回参数信息)
inline std::string PARSE_CMD(const char *msg, char *out_cmd)
{
	const char *p = msg;
	while (*p && ':' != *p) ++p;
	memcpy(out_cmd, msg, p - msg);
	return (':' == *p) ? p + 1 : "";
}

#define RESTART			"restart"

#define REFRESH			"refresh"

#define STOP			"stop"

#define START			"start"

#define SHUTDOWN		"shutdown"

#define REBOOT			"reboot"

#define REGISTER		"register"

#define KEEPALIVE		"keepAlive"

#define UPDATE			"update"

#define SETTIME			"settime"
