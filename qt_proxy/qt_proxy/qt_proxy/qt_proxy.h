#ifndef QT_PROXY_H
#define QT_PROXY_H

#include <QtWidgets/QWidget>
#include <QString>
#include <QThread>
#include <QTime>
#include "def.h"
#include "ui_qt_proxy.h"


class ProxyServer;
class WorkerThread;

class qt_proxy : public QWidget
{
	Q_OBJECT

public:
	qt_proxy(QWidget *parent = 0);
	~qt_proxy();

private slots:
	void slot_printLog(QString&, LogType type);
	void slot_clean(bool checked);
signals:
	void signal_printLog(QString&, LogType type);
private:
	
private:
	Ui::qt_proxyClass ui;	
	WorkerThread *m_thread = nullptr;
	qint64 m_totalData = 0;
	QTime m_lastTime;
};




class WorkerThread : public QThread
{
	Q_OBJECT
public:
	static void callBackPrintLog(void *, QString&, LogType type= STR_LOG);
	explicit WorkerThread(QObject *parent = 0)
		: QThread(parent)
	{}
protected:
	virtual void run() Q_DECL_OVERRIDE;
signals:
	void signal_PrintLog(QString&, LogType type);
private:
	void emitPrintLog(QString&, LogType type);
private:
	ProxyServer *m_proxyServer = nullptr;
};

#endif // QT_PROXY_H
