#ifndef CURLTESTSEVER_H
#define CURLTESTSEVER_H

#include <QtWidgets/QMainWindow>
#include "ui_curltestsever.h"

#include <QMap>

class CurlThread;
class curlTestSever : public QMainWindow
{
	Q_OBJECT

public:
	curlTestSever(QWidget *parent = 0);
	~curlTestSever();

	private slots:
	void slot_startBtnClicked();
	void slot_cleanBtnClicked();
	void slot_disStr(QString &,intptr_t);
	void slot_emitData(intptr_t,intptr_t);

private:
	Ui::curlTestSeverClass ui;

	CurlThread *m_pcurl = nullptr;
	QMap<intptr_t, QString> m_mapUrl;
};

#endif // CURLTESTSEVER_H
