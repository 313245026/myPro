#include "logs.h"
#include <QFile>
#include <QDateTime>
#include <QDir>

QString Logs::m_logName = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");

Logs::Logs(QObject *parent)
	: QObject(parent)
{

	logPath = QDir::currentPath() + "/" + m_logName + ".txt";
	//logPath = QDir::currentPath() + "/" + "321.txt";
	QDir curDir = QDir::current();
	QFile file(logPath);
	if (!file.exists())
	{
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		file.close();
	}

}

Logs::~Logs()
{
	writeToFile();
}

void Logs::writeToFile()
{
	if (m_strLog.size() > 1024 * 4)
	{
		QFile file(logPath);
		
		if (file.open(QIODevice::Append))
		{
			file.write(m_strLog.toLocal8Bit());
			file.close();
			m_strLog.clear();
		}
	}
}

void Logs::addLog(QString str)
{
	QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
	str = timeStr + str + "\n";
	m_strLog.append(str);
	writeToFile();
}

Logs&  Logs::GetInstance()
{
	static Logs log(NULL);
	return log;
}

