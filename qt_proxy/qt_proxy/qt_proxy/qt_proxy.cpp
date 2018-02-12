#include "qt_proxy.h"
#include "ProxyServer.h"


qt_proxy::qt_proxy(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_thread = new WorkerThread(this);
	m_thread->start();
	connect(m_thread, &WorkerThread::signal_PrintLog, this, &qt_proxy::slot_printLog,Qt::BlockingQueuedConnection);
	connect(ui.pushButton_2, &QPushButton::clicked, this, &qt_proxy::slot_clean);

	m_lastTime = QTime::currentTime();
}

qt_proxy::~qt_proxy()
{
}

void qt_proxy::slot_printLog(QString &str, LogType type)
{

	if (type == TOTAL)
	{
		double lenData = str.toInt();
		m_totalData += lenData;
		QString str = QString::number(m_totalData / 1024/1024 );
		ui.label_total->setText(str);

// 		QTime curTime = QTime::currentTime();
// 		double mse = m_lastTime.msecsTo(curTime);
// 		double nSpeed = (lenData /mse);
// 		ui.label_rcvSpeed->setText(0);
// 		m_lastTime = QTime::currentTime();
	}
	else
	{
		ui.log_textEdit->append(str);
	}
}

void qt_proxy::slot_clean(bool checked)
{
	ui.log_textEdit->clear();
}

void WorkerThread::callBackPrintLog(void *ptrObj, QString &strLog, LogType type)
{
	WorkerThread *proxy = static_cast<WorkerThread*>(ptrObj);
	if (proxy)
		proxy->emitPrintLog(strLog, type);
}

void WorkerThread::emitPrintLog(QString &strLog, LogType type)
{
	emit signal_PrintLog(strLog,  type);
}

void WorkerThread::run()
{
	m_proxyServer = new ProxyServer;
	m_proxyServer->SetCallBackInfo(callBackPrintLog, (void*)this);
	m_proxyServer->StartServer();
}