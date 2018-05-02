#pragma once
#include "../stdafx.h"

/// 缓冲区长度
#define BUFFER_LENGTH 4096

/// SIP头最长长度
#define MAX_SIPLENGTH 256

#define XML_TAG			"<?xml"			/**< XML标记 */

#define CALLID_TAG		"Call-ID"		/**< Callid标记 */

#define CSEQ_TAG		"CSeq"			/**< Cseq标记 */

#define XML_LEN_TAG		"Content-Length" /**< XML长度标记 */

typedef char str32[32];

// 获取seq序列及CallId
const char* GetSeqAndCallId(const char *buffer, str32 &seq, str32 &cid);

class RingBuffer
{
public:
	RingBuffer( int size );
	~RingBuffer(void);
public:

	int PushData(const char *pData, int size);
	int PopData(char *pData, int bufSize);
	int PopDataEx(char *pData, int bufSize);
	void Reset( );
	
private:
	int AddData(const char *pData, unsigned int length);
	int GetData(char *buf, unsigned int max);
	int GetDataEx(char *buf, unsigned int nLen);

public:

	// 返回当前缓冲区中的数据
	int Size() const { return m_totalData; };
	// 返回当前缓冲区中空闲空间的大小
	int Space() const { return m_maxSize-m_totalData; };
	//////////////////////////////////////////////////////////////////////////
	// 弹出一个完整的SipXml数据
	bool PopXml(char *pDst, const bool * = 0);// 该函数可能PopXml Sip头不符合规范
	// 获得一个完整的SipXml数据
	bool GetXml(char *pDst, const bool *bExitFlag);
	// 获得SipXml长度
	inline int GetSipXmlLength() const
	{
		return m_nSipXmlLen;
	}

private:
	unsigned int m_totalData;
	unsigned int m_readPointer;
	unsigned int m_writePointer;
	unsigned int m_maxSize;

	CRITICAL_SECTION m_section;
	char *m_buf;

	//////////////////////////////////////////////////////////////////////////
	static char m_xml[BUFFER_LENGTH];	/**< 当前SipXml[多线程不安全] */
	int m_nSipXmlLen;					/**< 当前SipXml长度 */

	bool m_bGotSip;						/**< 是否已取得Sip头 */
	char *m_pos;						/**< 指向当前sipXml字符串 */
	// 弹出一个完整的sipXML
	const char* _PopXml(int nSipLength = MAX_SIPLENGTH, int nStep = 1);
	int getXmlLenFromSip(char* cHead);
};
