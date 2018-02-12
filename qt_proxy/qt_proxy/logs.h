#ifndef LOGS_H
#define LOGS_H

#include <QObject>
#include <QString>

class Logs : public QObject
{
	Q_OBJECT

public:
	~Logs();
	static Logs&  GetInstance();

	void addLog(QString);
	void writeToFile();
private:
	Logs(QObject *parent);
	static QString m_logName;
	QString m_strLog = "";
	QString logPath = "";
};

#endif // LOGS_H
