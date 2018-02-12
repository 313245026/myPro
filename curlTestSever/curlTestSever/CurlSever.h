#pragma once


#include "curl7.21.3/include/curl/curl.h"
#include <iostream>
#include <QObject>
#include <QList>
#include <QMutex>
#include <QWaitCondition>
#include <QMap>

using namespace std;


class CurlSever :public QObject
{
	Q_OBJECT
public:
	CurlSever(QObject *parent = 0);
	~CurlSever();

	intptr_t Post(const std::string & sUrl,const std::string data, const std::string & sProxy, unsigned int uiTimeout);
	intptr_t Get(const std::string & sUrl, const std::string & sProxy,  unsigned int uiTimeout);

	intptr_t Gets(std::string & strUrl, const std::string & sProxy, char* szHost, const char * pCaPath);

	CURL * curl_easy_handler(const std::string & sUrl, const std::string & sProxy, unsigned int uiTimeout);
	int curl_multi_select(CURLM * curl_m);
	int curl_multi_demo();
	int curl_easy_demo(int num);

	int MultiSelect(void);

signals:
	void signal_dispStr(QString &,intptr_t);
	void signal_emitData(intptr_t PtrStr, intptr_t Ptrcurl);

private:
	CURLM *m_pMultiHandle = nullptr;	
	QList<CURL*> m_listCurl;
	QMutex *m_mutex;
	QMutex *m_mutex2;
	QWaitCondition m_waitCondition;
	QMap<intptr_t, std::string*> m_mapCurlStr;
};

