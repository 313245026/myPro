#include "performance.h"
#include <QDateTime>

#include "logs.h"

QMap<QString, int> Performance::m_funcCount = QMap<QString, int>();

Performance::Performance(QObject *parent,QString fncName)
	: QObject(parent)
{
	m_oldMs = QDateTime::currentMSecsSinceEpoch();
	m_fncName = fncName;

	if (m_funcCount.find(m_fncName) != m_funcCount.end())
		m_funcCount[m_fncName] = m_funcCount[m_fncName] + 1;
	else
		m_funcCount.insert(m_fncName, 1);
}

Performance::~Performance()
{
	qint64 difMs = QDateTime::currentMSecsSinceEpoch() - m_oldMs;
	if (difMs == 0)
		return;
	QString strLog = QString("%3 used count = %1 ,cost ms time %2").arg(m_funcCount[m_fncName]).arg(difMs).arg(m_fncName);	
	Logs::GetInstance().addLog(strLog);
}
