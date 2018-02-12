#ifndef CURLTHREAD_H
#define CURLTHREAD_H

#include <QThread>
class CurlSever;

class CurlThread : public QThread
{
	Q_OBJECT

public:
	CurlThread(QObject *parent);
	~CurlThread();

	intptr_t Post(const std::string & sUrl, const std::string data, const std::string & sProxy,  unsigned int uiTimeout);
	intptr_t Get(const std::string & sUrl, const std::string & sProxy, unsigned int uiTimeout);
	intptr_t Gets(std::string & strUrl, const std::string & sProxy, char* szHost, const char * pCaPath);

	int GetReqUrlCount();

	void stop();
	void run();

public slots:
	void slot_TdispStr(QString &,intptr_t);

signals:
	void signal_TdispStr(QString &, intptr_t);
	void signal_emitData(intptr_t PtrStr, intptr_t Ptrcurl);

private:
	CurlSever *m_curlSever;
	bool m_isStop = false;
};

#endif // CURLTHREAD_H
