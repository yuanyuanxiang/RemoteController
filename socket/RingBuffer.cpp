#include "RingBuffer.h"
#include <stdlib.h>
#include <assert.h>
#include <string>

#ifndef OutputDebugInfo
#define OutputDebugInfo printf
#endif

#define	BCECR_MAX_FRAME_SIZE		(1024 * 1024)
#define MAX_READ_SIP_HEADER_LEN			256

char RingBuffer::m_xml[BUFFER_LENGTH] = { 0 };

/**
* @brief 获取seq序列及CallId
* @param[in] &seq seq序列
* @param[in] &cid CallId
* @return 返回xml
*/
const char* GetSeqAndCallId(const char *buffer, str32 &seq, str32 &cid)
{
	if (NULL == buffer || strlen(buffer) <= MAX_SIPLENGTH)
		return "";
	const char *Cid = strstr(buffer, CALLID_TAG);
	if (!Cid) return "";
	const char *CSeq = strstr(buffer, CSEQ_TAG);
	if (!CSeq) return "";
	const char *xml = strstr(buffer, XML_TAG);
	if (!xml) return "";

	// 获得CallId及CSeq
	memset(cid, 0, 32);
	memset(seq, 0, 32);
	while (*Cid != ':' && *Cid) ++Cid;
	if (*Cid == ':') ++Cid;
	while (*Cid == ' ') ++Cid;// 定位到CallId
	while (*CSeq != ':' && *CSeq) ++CSeq;
	if (*CSeq == ':') ++CSeq;
	while (*CSeq == ' ') ++CSeq;// 定位到Cseq
	memcpy(cid, Cid, 22);
	memcpy(seq, CSeq, 8);
	char *p = seq + 8;// 从后往前确定seq的尾巴
	while (*p != ' ' && *p != *seq) --p; *p = 0;

	return xml;
}

RingBuffer::RingBuffer( int size )
{
	m_maxSize = size;
	m_buf = ( char* )new char[size];
	if( m_buf == NULL )
	{
		OutputDebugStringA("============exit because failed to malloc memory===========\n");
		exit(0);
	}

	m_totalData    = 0;;
	m_readPointer  = 0;
	m_writePointer = 0;
	m_bGotSip = false;
	m_nSipXmlLen = 0;
	m_pos = m_xml;

	InitializeCriticalSection( &m_section );
	
}

RingBuffer::~RingBuffer(void)
{
	if( m_buf != NULL )
	{
		delete[] m_buf;
		m_buf = NULL;
	}

	DeleteCriticalSection( &m_section );
}

int RingBuffer::AddData( const char *pData, unsigned int length )
{

	EnterCriticalSection( &m_section );

	if( m_totalData + length > m_maxSize )
	{
		LeaveCriticalSection( &m_section );
		return 0;
	}   

	if( m_writePointer + length <= m_maxSize )
	{
		memcpy( m_buf + m_writePointer, pData, length);
		m_writePointer += length;

	}else
	{
		memcpy( m_buf + m_writePointer, pData, m_maxSize - m_writePointer );
		memcpy( m_buf, ( pData + m_maxSize - m_writePointer ), length -( m_maxSize - m_writePointer ) );
		m_writePointer = length - ( m_maxSize - m_writePointer );
	}

	m_totalData += length;

	LeaveCriticalSection( &m_section );

	return length;

}

int RingBuffer::GetData( char *buf, unsigned int max )
{
	do{

		if( m_totalData < 1 )
		{
			break;
		}
		
		unsigned int length = max;

		if(  length > m_totalData )
		{
			//Error accured and i reset all pointers.
			m_readPointer    = 0;
			m_writePointer   = 0;
			m_totalData  = 0;
			break;
		}

		if( m_readPointer + length <= m_maxSize )
		{
			memcpy( buf, m_buf + m_readPointer, length );
			m_readPointer += length;
		}else
		{
			memcpy( buf, m_buf + m_readPointer, m_maxSize - m_readPointer );
			memcpy( buf + m_maxSize - m_readPointer,
				m_buf, length + m_readPointer - m_maxSize );
			m_readPointer = length + m_readPointer - m_maxSize;
		}

		m_totalData -= ( length );
		return length;

	}while( 0 );

	return 0;
}

int RingBuffer::GetDataEx(char *buf, unsigned int nLen)
{
	do{

		if( m_totalData < 1 )
		{
			break;
		}

		unsigned int length = nLen;
		if(  length > m_totalData )
		{
			length = m_totalData;
		}

		if( m_readPointer + length <= m_maxSize )
		{
			memcpy( buf, m_buf + m_readPointer, length );
			m_readPointer += length;
		}else
		{
			memcpy( buf, m_buf + m_readPointer, m_maxSize - m_readPointer );
			memcpy( buf + m_maxSize - m_readPointer,
				m_buf, length + m_readPointer - m_maxSize );
			m_readPointer = length + m_readPointer - m_maxSize;
		}

		m_totalData -= ( length );
		return length;

	}while( 0 );

	return 0;
}


void RingBuffer::Reset()
{
	EnterCriticalSection( &m_section );

	m_totalData = 0;
	m_writePointer = 0;
	m_readPointer = 0;
	m_bGotSip = false;
	m_nSipXmlLen = 0;
	m_pos = m_xml;

	LeaveCriticalSection( &m_section );

}

/**
* @brief 向环形缓存区增加数据
* @param[in] *pData 待增加数据
* @param[in] size 数据的长度
* @return 返回数据长度
* @note size = 0时不做任何操作
*/
int RingBuffer::PushData( const char *pData, int size )
{
	if (size)
	{
//#ifdef _DEBUG
//		char buffer[BUFFER_LENGTH+4];
//		memcpy(buffer, pData, size);
//		buffer[size] = 0;
//		printf(buffer);
//#endif
	}
	return size ? AddData( pData, size ) : 0;
}

int RingBuffer::PopData( char *pData, int bufSize )
{
	int ret = 0;
	EnterCriticalSection( &m_section );

	do
	{
		if( pData == NULL )
		{
			break;
		}

		if (bufSize > m_totalData)
		{
			break;
		}
		
		ret = GetData( pData, bufSize);

	}while( false );

	LeaveCriticalSection( &m_section );
	
	return ret;		
}


int RingBuffer::PopDataEx( char *pData, int bufSize )
{
	int ret = 0;
	EnterCriticalSection( &m_section );

	do
	{
		if( pData == NULL )
		{
			break;
		}

		ret = GetDataEx( pData, bufSize);

	}while( false );

	LeaveCriticalSection( &m_section );

	return ret;
}

// 弹出一个完整的SipXml数据
bool RingBuffer::PopXml(char *pDst, const bool *)
{
	EnterCriticalSection(&m_section);
	const char *poped = _PopXml(MAX_SIPLENGTH);
	if(poped) memcpy(pDst, poped, m_nSipXmlLen);
	pDst[m_nSipXmlLen] = 0;
	LeaveCriticalSection(&m_section);
	return poped && m_nSipXmlLen > MAX_SIPLENGTH;
}


/**
* @brief 弹出一个XML
* @param[in] nSipLength SIP头长度[256]
* @param[in] nStep 迭代步数
* @return 成功返回字符串，失败返回0
*/
const char* RingBuffer::_PopXml(int nSipLength, int nStep)
{
	// 最大递归深度
	const int STEP = BUFFER_LENGTH / MAX_SIPLENGTH;
	// 防止无穷递归
	if (nStep > STEP)
	{
		OutputDebugInfo("PopXml 递归次数过大!");
		OutputDebugInfo(m_xml);// 输出当前XML
		m_bGotSip = false;
		m_pos = m_xml;
		m_nSipXmlLen = 0;
		return NULL;
	}
	
	if (m_bGotSip)// 已获取SIP头
	{
		if (PopData(m_pos, nSipLength))
		{
			m_bGotSip = false;
			const char* ret = m_pos - m_nSipXmlLen;
			m_nSipXmlLen += nSipLength;
			m_pos = m_xml;
			return ret;
		}
		m_nSipXmlLen = 0;
		return NULL;
	}
	else// 未获取SIP头
	{
		memset(m_pos, 0, strlen(m_pos));
		// 如果获取SIP头成功
		if (PopData(m_pos, nSipLength))
		{
			m_nSipXmlLen = nSipLength;
			const char *xmllen = strstr(m_xml, XML_LEN_TAG);// xml长度的位置
			if (NULL == xmllen)// 弹出一个Sip头，但是不包含xml长度的字符串[这是有问题的]
			{
				OutputDebugInfo("PopXml Sip头不符合规范!");
				if (strstr(m_xml, "INVITE sip") || strstr(m_xml, "SIP/2.0 200"))
				{
					m_pos += nSipLength;// 包含sip头将继续取数据
					return _PopXml(nSipLength, nStep + 1);
				}
				else 
				{
					m_pos = m_xml;
					m_nSipXmlLen = 0;
					return NULL;
				}
			}
			m_pos += nSipLength;
			while (*xmllen != ':' && *xmllen) ++xmllen;// 找冒号
			if (*xmllen == ':') ++xmllen;// 跳过冒号
			while (*xmllen == ' ') ++xmllen;// 定位到xml长度的位置
			char *xmlhead = strstr(m_xml, XML_TAG);// xml头的位置
			int left = m_pos - xmlhead;// 已收xml包长度
			char num[8] = { 0 };
			memcpy(num, xmllen, 7);
			int nLeftXmlLen = atoi(num) - left;
			m_bGotSip = true;
			return _PopXml(nLeftXmlLen, nStep + 1);
		}
		m_nSipXmlLen = 0;
		return NULL;
	}
}

/**
* @brief 弹出一个XML
* @param[in] pDst XML
* @param[in] bExitFlag 条件
* @return 成功返回true，失败返回false
*/
bool RingBuffer::GetXml(char *pDst, const bool *bExitFlag)
{
	// 获取SIP
	*pDst = 0;
	if (PopData(pDst, MAX_SIPLENGTH) <= 0)
		return false;
	pDst[MAX_SIPLENGTH] = 0;
	int length = getXmlLenFromSip(pDst);
	if (length <= 0)
	{
		printf(pDst);
		OutputDebugInfo("Parse error : getXmlLenFromSip <= 0! \n");
		*pDst = 0;
		return false;
	}

	// 获取XML
	char *pXml = strstr(pDst, XML_TAG);
	if (NULL == pXml)
	{
		printf(pDst);
		OutputDebugInfo("Parse error : buffer not has a xml! \n");
		*pDst = 0;
		return false;
	}
	int xmlLen = strlen(pXml);
	int nLeftSize = length - xmlLen;
	char cXmlBuf[BUFFER_LENGTH] = { 0 };
	strcpy_s(cXmlBuf, pXml);

	pXml = cXmlBuf + xmlLen;
	int nPopSize = 0;
	do 
	{
		nPopSize = PopData(pXml, nLeftSize);
		if (nPopSize == nLeftSize)
			break;
		Sleep(10);
	} while (NULL == bExitFlag || false == *bExitFlag);
	pXml[nPopSize] = 0;
	strcat(pDst, pXml);
	m_nSipXmlLen = MAX_SIPLENGTH + nPopSize;
	if (nPopSize == nLeftSize)
		return true;
	*pDst = 0;
	return false;
}

void string_replace(std::string &s1, const std::string s2, const std::string s3)
{
	if (s2.empty())
		return;

	std::string::size_type pos = 0;
	std::string::size_type a = s2.size();
	std::string::size_type b = s3.size();
	while((pos = s1.find(s2, pos)) != std::string::npos)
	{
		s1.replace(pos, a, s3);
		pos += b;
	}
}

int RingBuffer::getXmlLenFromSip(char* cHead)
{
	int len = 0;
	char *p = strstr(cHead, XML_LEN_TAG);
	if (NULL != p)
	{
		p = p + strlen(XML_LEN_TAG) + 1;
		char *pXml = strstr(cHead, XML_TAG);
		if (pXml!=NULL)
		{
			len = pXml - p;

			char val[32] = {0};
			//for
			strncpy(val, p, len);

			std::string strValue = val;
			string_replace(strValue, " ", "");

			len = atoi(strValue.c_str());
		}
	}
	return len;
}
