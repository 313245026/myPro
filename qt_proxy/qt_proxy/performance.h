#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <QObject>
#include <QMap>


class Performance : public QObject
{
	Q_OBJECT

public:
	Performance(QObject *parent,QString fncnName);
	~Performance();

private:
	qint64 m_oldMs = 0;
	QString m_fncName = "";
	static QMap<QString, int> m_funcCount;
};

#define COUNT_PERFORMANCE Performance perfor(nullptr,QString(__FUNCTION__));
#endif // PERFORMANCE_H
