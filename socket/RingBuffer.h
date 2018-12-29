#pragma once

#include <windows.h>

/************************************************************************
* @class RingBuffer
* @update 袁沅祥,2018-10-19
************************************************************************/
class RingBuffer
{
public:
	RingBuffer(unsigned int size);
	~RingBuffer(void);

public:

	inline int PushData(const char *pData, int size) { return AddData(pData, size); }

	inline int PushFrame(const char *pFrame, int size) { return AddData(pFrame, size); }

	inline void Reset() { EnterCriticalSection(&m_section);ResetData();LeaveCriticalSection(&m_section); }

	inline int GetFrameNum() const { return m_frameNum; }

	int PopData(char *pData, unsigned int bufSize);
	
	int PopAllData(char *pData, unsigned int bufSize);

private:

	int AddData(const char *pData, unsigned int length);

	int GetData(char *buf, unsigned int length);

	// 当total>m_maxSize时进行扩容
	int raise(unsigned int total);

	// 当total<m_maxSize/2时缩小容量
	void reduce(unsigned int size);

	inline void ResetData() { m_totalData = 0; m_writePointer = 0; m_readPointer = 0; m_frameNum = 0; }

private:
	unsigned int m_totalData;			// 当前总字节
	unsigned int m_readPointer;			// 读指针
	unsigned int m_writePointer;		// 写指针
	unsigned int m_curSize;				// 当前容量
	unsigned int m_minSize;				// 最小容量（初始值）
	unsigned int m_frameNum;			// 帧数

	CRITICAL_SECTION m_section;
	char *m_buf;
};
