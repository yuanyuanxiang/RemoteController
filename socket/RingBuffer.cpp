#include "RingBuffer.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <new>

// 循环存储区的最大容量（2Mb）
#define MAX_BUFFER (2<<20)

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete [] (p); (p)=NULL; }
#endif

#ifndef LOG_AND_PRINT
#define LOG_AND_PRINT(p) printf(p)
#endif

#ifndef MY_LOG_INFO
#define MY_LOG_INFO(p) printf("%s \n", p)
#endif

RingBuffer::RingBuffer(unsigned int size)
{
	InitializeCriticalSection(&m_section);

	m_curSize = m_minSize = size;
	ResetData();

	try{
		m_buf = new char[m_curSize]();
	} catch(std::bad_alloc &e){
		m_buf = NULL;
		MY_LOG_INFO(e.what());
		throw(e.what());
	}
}

RingBuffer::~RingBuffer(void)
{
	SAFE_DELETE_ARRAY(m_buf);

	DeleteCriticalSection(&m_section);
}

// 扩容成功返回1, 否则返回0
int RingBuffer::raise(unsigned int total)
{
	unsigned int result = 0, m = m_curSize;
	do { m *= 2; } while (m < total);
	if (m <= MAX_BUFFER)
	{
		char *pNew = NULL;
		try { pNew = new char[m](); }
		catch (std::bad_alloc &e){ MY_LOG_INFO(e.what()); pNew = NULL; }
		if (pNew)
		{
			unsigned int totalData = m_totalData;
			GetData(pNew, totalData); // 拷贝数据
			char s[64];
			sprintf_s(s, "[INFO]RingBuffer扩容: %dMb===>%dMb.\n", m_curSize>>20, m>>20);
			LOG_AND_PRINT(s);
			SAFE_DELETE_ARRAY(m_buf); m_buf = pNew; m_curSize = m;
			m_readPointer = 0; m_writePointer = m_totalData = totalData;
			result = 1;
		}
	}
	return result;
}

void RingBuffer::reduce(unsigned int size)
{
	unsigned int m = size;
	char *pNew = NULL;
	try { pNew = new char[m](); }
	catch (std::bad_alloc &e){ MY_LOG_INFO(e.what()); pNew = NULL; }
	if (pNew)
	{
		unsigned int totalData = m_totalData;
		GetData(pNew, totalData); // 拷贝数据
		char s[64];
		sprintf_s(s, "[INFO]RingBuffer缩容: %dMb===>%dMb.\n", m_curSize>>20, m>>20);
		LOG_AND_PRINT(s);
		SAFE_DELETE_ARRAY(m_buf); m_buf = pNew; m_curSize = m;
		m_readPointer = 0; m_writePointer = m_totalData = totalData;
	}
}

int RingBuffer::AddData(const char *pData, unsigned int length)
{
	EnterCriticalSection(&m_section);
	unsigned int total = m_totalData + length;
	if(total > m_curSize && 0 == raise(total))
	{
		LeaveCriticalSection(&m_section);
		return 0;
	}
	if(m_writePointer + length <= m_curSize)
	{
		memcpy(m_buf + m_writePointer, pData, length);
		m_writePointer += length;

	}else
	{
		unsigned int left = m_curSize - m_writePointer;
		memcpy(m_buf + m_writePointer, pData, left);
		m_writePointer = length - left;
		memcpy(m_buf, pData + left, m_writePointer);
	}
	++m_frameNum;
	m_totalData = total;
	if (m_minSize < m_curSize && total < m_curSize/4)
		reduce(m_curSize/2);
	LeaveCriticalSection(&m_section);

	return length;
}

int RingBuffer::GetData(char *buf, unsigned int length)
{
	do{
		if(0 == m_totalData)
			break;

		if(length > m_totalData)//Error accured and i reset all pointers.
		{
			m_readPointer    = 0;
			m_writePointer   = 0;
			m_totalData      = 0;
			break;
		}

		if(m_readPointer + length <= m_curSize)
		{
			memcpy(buf, m_buf + m_readPointer, length);
			m_readPointer += length;
		}else
		{
			unsigned int left = m_curSize - m_readPointer;
			memcpy(buf, m_buf + m_readPointer, left);
			m_readPointer = length - left;
			memcpy(buf + left, m_buf, m_readPointer);
		}

		m_totalData -= length;
		return length;
	}while(false);

	return 0;
}

int RingBuffer::PopData(char *pData, unsigned int bufSize)
{
	assert(pData);
	EnterCriticalSection(&m_section);
	int ret = bufSize <= m_totalData ? GetData(pData, bufSize) : 0;
	LeaveCriticalSection(&m_section);

	return ret;		
}

int RingBuffer::PopAllData(char *pData, unsigned int bufSize)
{
	assert(pData);
	
	EnterCriticalSection(&m_section);
	int getdatalen = 0;
	if(bufSize <= m_totalData)
	{
		getdatalen = bufSize;
	}
	else
	{
		getdatalen = m_totalData;
	}		
	int ret = GetData(pData, getdatalen);
	LeaveCriticalSection(&m_section);

	return ret;		
}
