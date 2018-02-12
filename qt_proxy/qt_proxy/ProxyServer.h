#pragma once


/*
纯转发服务-使用多路复用-单线程
*/
#include "def.h"

#include <QString>
#include <QList>
#include <QMap>
#include <WS2tcpip.h>  
#include <WinSock2.H>  
#include <QMap>
#pragma comment(lib, "ws2_32.lib") 

typedef void(*PFunCallBackPrintLog)(void *ptrObj, QString &strLog, LogType type);
struct IPData 
{
	string _ip;
	short ipType;
};
class ProxyServer
{
	
public:
	ProxyServer();
	virtual ~ProxyServer();
	void SetCallBackInfo(PFunCallBackPrintLog,void*);

	void SetProxyInfo();
	bool StartServer();

private:
	void printLog(QString &,LogType type = STR_LOG);
	bool preSocketInit(UINT_PTR &socketListen);
	bool getAcceptSocket(UINT_PTR socketListen,UINT_PTR &acceptSocket);
	bool endSocket();
	bool processReadSocket( fd_set readSet);
	bool processWriteSocket( fd_set writeSet);
	bool processListenSocket(UINT_PTR socke,fd_set &readSet);
	bool connectToHttpServer(socketPair& ,string &);	
	bool sendData(UINT_PTR socket,string&);
	int rcvSocketData(UINT_PTR socket,string &rcvStr);
	bool getHost(string &rcvData,string &hostName);
	void ClearSocketPair();
	bool setSocketFIONBIO(UINT_PTR );
	void newSocketToHttp(socketPair&,string &);
	bool httpSocketConnectSuc(socketPair&);
	string getConnectPort(const string &rcvStr);
	IPData getIpByHostName(string);

	void recoderRcvLen(int len);

	string getHeadByKey(string key,string rcvData);
	
private:
	PFunCallBackPrintLog m_pCallBackFunction = nullptr;
	void *m_widgetObj = nullptr;
	QList<socketPair> m_listSocketPair;
	
	fd_set m_allSet;
	fd_set m_writeSet;
	UINT_PTR m_listenSocket = 0;

	int m_rcvCount = 0;
	string m_httpsResp;
	QMap<string, IPData> m_mapHostNameIp;
};

