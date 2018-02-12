#include "curlthread.h"

#include "CurlSever.h"

CurlThread::CurlThread(QObject *parent)
	: QThread(parent)
{
	m_curlSever = new CurlSever(this);
	connect(m_curlSever, &CurlSever::signal_dispStr, this, &CurlThread::signal_TdispStr);
	connect(m_curlSever, &CurlSever::signal_emitData, this, &CurlThread::signal_emitData);
}

CurlThread::~CurlThread()
{
	if (m_curlSever)
	{
		delete m_curlSever;
		m_curlSever = nullptr;
	}	
}

void CurlThread::slot_TdispStr(QString &str, intptr_t pt)
{
	emit signal_TdispStr(str,pt);
}

int CurlThread::GetReqUrlCount()
{
	return 10;
}

void CurlThread::stop()
{
	//stop Ïß³Ì
	m_isStop = true;
}

void CurlThread::run()
{
	while (true)
	{
		if (m_isStop)
			break;
		m_curlSever->curl_multi_demo();
	}
	
}

intptr_t CurlThread::Post(const std::string & sUrl, const std::string data, const std::string & sProxy, unsigned int uiTimeout)
{
	return m_curlSever->Post(sUrl,data,sProxy,uiTimeout);
}
intptr_t CurlThread::Get(const std::string & sUrl, const std::string & sProxy,  unsigned int uiTimeout)
{
	return m_curlSever->Get(sUrl, sProxy,  uiTimeout);
}

intptr_t CurlThread::Gets(std::string & strUrl, const std::string & sProxy, char* szHost, const char * pCaPath)
{
	return m_curlSever->Gets(strUrl, sProxy, szHost, pCaPath);
}