#include "curltestsever.h"
#include <QMetaType>

#include "curlthread.h"

#include <windows.h>
#include <QLineEdit>
#include <QPushButton>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QUrl>

#include <QDateTime>

#include <string>
using namespace std;

//Ŀǰ�ƻ�
//����ͨ�����ؿ⣬����http�� ����socket�� ʹ�ó������ڿͻ���ʹ�� �������ɴ�C++�ģ�������ֲ
//���̣߳��첽������

curlTestSever::curlTestSever(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//ui.textEdit_log		ui.lineEdit_port  ui.lineEdit_IP ui.pushButton_start ui.pushButton_clear ui.label_reqCount
	connect(ui.pushButton_start,&QPushButton::clicked,this, &curlTestSever::slot_startBtnClicked);
	connect(ui.pushButton_clear, &QPushButton::clicked, this, &curlTestSever::slot_cleanBtnClicked);
	m_pcurl = new CurlThread(this);
	m_pcurl->start();
	qRegisterMetaType<QString>("QString&");
	qRegisterMetaType<QString>("QString");
	qRegisterMetaType<intptr_t>("intptr_t");
	connect(m_pcurl, &CurlThread::signal_TdispStr, this, &curlTestSever::slot_disStr);
	connect(m_pcurl, &CurlThread::signal_emitData, this, &curlTestSever::slot_emitData);
	
}

curlTestSever::~curlTestSever()
{

}

void curlTestSever::slot_startBtnClicked()
{
	QString     strUrl = "127.0.0.1:8080";
	//std::string     URL = "https://www.baidu.com"; //"http://website.com";		// ������������Ҫ���ʵ�url // 
	//std::string     URL = "http://5b0988e595225.cdn.sohucs.com/images/20171111/79e0910a8b9048119bed0ad6e497fbc1.jpeg";
	std::string     PROXY = "";//"ip:port";										 // �������ô���ip�Ͷ˿�  //
	unsigned int    TIMEOUT = 5000; /* ms */// �������ó�ʱʱ��  //
	int Ncount = ui.lineEdit_count->text().toInt();

	//get all jpg
	QFile file("url.txt");
	QStringList listUrl;
	if (file.open(QIODevice::ReadOnly))
	{
		QString fileTxt = file.readAll();
		listUrl = fileTxt.split("\n");
	}
	QDir dir;
	dir.mkdir("dir_jpg");
	//QString strUrl( URL.data());
	for (int i = 0; i < 50000; i++)
	{
// 		QString strUrl = ite;
// 		strUrl = strUrl.replace("\r", "");
// 		if (strUrl == "")
// 			continue;
// 		if (i % 50000)
// 			Sleep(1);
		intptr_t pt = m_pcurl->Get(strUrl.toStdString(), "",9000);

	//	m_mapUrl.insert(pt, strUrl);
		//break;
	}
}

void curlTestSever::slot_cleanBtnClicked()
{
	ui.textEdit_log->clear();
}

void curlTestSever::slot_disStr(QString &str, intptr_t ptr)
{
 //	ui.textEdit_log->append(str);
}

void curlTestSever::slot_emitData(intptr_t PtrData, intptr_t PtrCurl)
{
	return;
	ui.textEdit_log->append(QDateTime::currentDateTime().toString("h:m:s:zz"));

	if (m_mapUrl.find(PtrCurl) != m_mapUrl.end())
	{
		QString strUrl = m_mapUrl[PtrCurl];
		QUrl url(strUrl);
		QString fileName = url.fileName();
		QString strCurl = QString("%1").arg(PtrCurl);
		QString strTime = QDateTime::currentDateTime().toString("sszzz");
		QString dirPath = QDir::currentPath() + "\\dir_jpg\\" + strCurl + strTime + fileName;
		string *str = (string*)PtrData;
		QFile file(dirPath);

		QString qs = QString("%1").arg(str->size());
		ui.textEdit_log->append(qs);
		if (str->size() == 0)
		{
			ui.textEdit_log->append(strUrl);
		}

		file.open(QIODevice::WriteOnly);
		file.write(str->data(), str->size());
		file.close();

		delete str; str = nullptr;
	}
}