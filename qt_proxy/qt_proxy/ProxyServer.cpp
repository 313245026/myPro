

#include "ProxyServer.h"
#include "performance.h"

#include <ctime>
#include <QDateTime>

ProxyServer::ProxyServer()
{
	COUNT_PERFORMANCE;
	m_listSocketPair.clear();
	FD_ZERO(&m_writeSet);
	FD_ZERO(&m_allSet);
	m_httpsResp = "HTTP/1.1 200 Connection Established\r\n\r\n";
}

ProxyServer::~ProxyServer()
{
	COUNT_PERFORMANCE;
	if (m_listSocketPair.count()>0)
	{
		for (auto ite: m_listSocketPair)
		{
			closesocket(ite.clientSocket.socket);
			closesocket(ite.httpSocket.socket);
		}
	}
	m_listSocketPair.clear();
}

void ProxyServer::SetCallBackInfo(PFunCallBackPrintLog callFuntion, void* widgetObj)
{
	m_widgetObj = widgetObj;
	m_pCallBackFunction = callFuntion;
}

void ProxyServer::printLog(QString &strLog, LogType type)
{
	m_pCallBackFunction(m_widgetObj, strLog, type);
	strLog = "";
}

void ProxyServer::SetProxyInfo()
{}

bool ProxyServer::preSocketInit(UINT_PTR &socketListen)
{
	QString strLog = "";
	WSADATA wsaData;
	WORD version = MAKEWORD(2, 2);
	int result = 0;
	result = WSAStartup(version, &wsaData);
	if (result != 0)
	{
		printLog(QString("WSAStartup() error."));
		return false;
	}

	/// 创建socket   
	
	socketListen = socket(AF_INET, SOCK_STREAM, 0);
	if (socketListen == INVALID_SOCKET)
	{
		printLog(QString("socket() error."));
		goto endFalse;
	}

	/// 服务器地址结构   
	sockaddr_in svrAddress;
	svrAddress.sin_family = AF_INET;
	svrAddress.sin_addr.s_addr = INADDR_ANY;
	svrAddress.sin_port = htons(8111);

	/// 绑定服务器套接字   
	result = bind(socketListen, (sockaddr*)&svrAddress, sizeof(svrAddress));
	if (result == SOCKET_ERROR)
	{
		closesocket(socketListen);
		printLog(QString("bind() error."));
		goto endFalse;
	}

	/// 开启监听  
	result = listen(socketListen, 5);
	if (result == SOCKET_ERROR)
	{
		closesocket(socketListen);
		printLog(QString("listen() error."));
		goto endFalse;
		
	}
	strLog = QString("bind:%1").arg(ntohs(svrAddress.sin_port));
	printLog(strLog);
	return true;

endFalse:
	WSACleanup();
	return false;
}

bool ProxyServer::getAcceptSocket(UINT_PTR socketListen, UINT_PTR &clientSocket)
{
	sockaddr_in clientAddr;
	int len = sizeof(clientAddr);

	clientSocket = accept(socketListen, (sockaddr*)&clientAddr, &len);
	if (clientSocket == INVALID_SOCKET)
	{
		printLog(QString("accept() error."));
		return false;
	}

	char ipAddress[16] = { 0 };
	inet_ntop(AF_INET, &clientAddr, ipAddress, 16);
	QString strLog = QString("Accept new connect[%1:%2]").arg(ipAddress).arg(ntohs(clientAddr.sin_port));
	//printLog(strLog);
	return true;
}

bool ProxyServer::endSocket()
{
	COUNT_PERFORMANCE;
	for (u_int i = 0; i < m_allSet.fd_count; ++i)
	{
		SOCKET socket = m_allSet.fd_array[i];
		closesocket(socket);
	}
	FD_ZERO(&m_allSet);
	WSACleanup();
	printLog(QString("endServer"));
	return true;
}

int ProxyServer::rcvSocketData(UINT_PTR socket, string &rcvStr)
{
	int result = -1;
	char bufRecv[4096];
	result = recv(socket, bufRecv, 4096, 0);
	if (result == SOCKET_ERROR)
	{
		DWORD err = WSAGetLastError();
		//if (err == WSAECONNRESET)       /// 客户端的socket没有被正常关闭,即没有调用closesocket 			
		return result;
	}
	else if (result == 0)               /// 客户端的socket调用closesocket正常关闭  
		return result;
	if (result > 0)
		rcvStr.append(bufRecv, result);
	return result;
}

string ProxyServer::getHeadByKey(string key, string rcvData)
{
	string strRet = "";
	size_t index1 = rcvData.find(key);
	if (index1 != string::npos)
	{
		int index2 = rcvData.find("\r\n", index1);
		string subStr = rcvData.substr(index1, index2 - index1);
		strRet = subStr.substr(key.size()+1);
	}
	return strRet;
}

bool ProxyServer::getHost(string &rcvStr, string &hostName)
{
	string strHost = getHeadByKey("Host:",rcvStr);
	string Origin = getHeadByKey("Origin:", rcvStr);
	string GetStr = getHeadByKey("GET", rcvStr);
	if (Origin == "")
		hostName = strHost;
	else
	{
		if (strHost.find("127.0.0.1") != string::npos)
		{
			string preHttp = "http://www.";
			size_t index = Origin.find(preHttp);
			if (index != string::npos)
			{
				hostName = Origin.substr(preHttp.size());
			}
		}	
	}
	
	if (GetStr!="" && Origin!=""&&GetStr.find("127.0.0.1") != string::npos)
	{
		size_t index = GetStr.find('?');
		string old = GetStr.substr(0, index-1);
		index = rcvStr.find(old);
		rcvStr.replace(index, old.size(), Origin);

	}
	return hostName != "";
	//这里有很多http协议需要处理
}

bool ProxyServer::setSocketFIONBIO(UINT_PTR ConnectSocket)
{
	unsigned long ul = 1;
	int res = ioctlsocket(ConnectSocket, FIONBIO, &ul); //设置为非阻塞模式
	if (res != 0)
	{
		return 1;
	}
	return res == 0;
}
string ProxyServer::getConnectPort(const string &rcvStr)
{
	size_t index1 = rcvStr.find("CONNECT");
	size_t index2 = rcvStr.find("\r\n", index1);
	if (index1 != -1 || index2 != -1)
	{
		string strLine = rcvStr.substr(index1, index2 - index1);
		size_t ind1 = strLine.find(":");
		size_t ind2 = strLine.find(" ", ind1);
		string strPort = strLine.substr(ind1 + 1, ind2 - ind1);
		//USHORT port = atoi(strPort.c_str());
		return strPort;
	}
	return "";
}

IPData ProxyServer::getIpByHostName(string hostName)
{
	if (m_mapHostNameIp.find(hostName) != m_mapHostNameIp.end())
		return m_mapHostNameIp[hostName];

	qint64 m_oldMs = QDateTime::currentMSecsSinceEpoch();
	hostent* phost = gethostbyname(hostName.c_str());
	qint64 difMs = QDateTime::currentMSecsSinceEpoch() - m_oldMs;
	if (!phost)
	{
		QString strLog = QString("*********%1--get IP faile--cost ms time = %2").arg(hostName.c_str()).arg(difMs);
		printLog(strLog);
		return IPData();
	}
	for (int i = 0; phost->h_addr_list[i]; i++)
	{
		char str[32] = { 0 };
		IPData _ipdata;
		string str_ip = inet_ntop(phost->h_addrtype, phost->h_addr_list[i], str, sizeof(str) - 1);
		_ipdata._ip = str_ip;
		_ipdata.ipType = phost->h_addrtype;
		m_mapHostNameIp.insert(hostName, _ipdata);
		break;
	}
	if (m_mapHostNameIp.find(hostName) != m_mapHostNameIp.end())
	{
		if (difMs > 5)
		{
			QString strLog = QString("%1--%2--cost ms time = %3").arg(hostName.c_str())
				.arg(m_mapHostNameIp[hostName]._ip.c_str()).arg(difMs);
			printLog(strLog);
		}			
		return m_mapHostNameIp[hostName];
	}		
	else
	{
		QString strLog = QString("%1--get IP faile--cost ms time = %2").arg(hostName.c_str()).arg(difMs);
		printLog(strLog);
		return IPData();
	}		
}

bool ProxyServer::connectToHttpServer(socketPair& sockPair, string &rcvStr)
{
	COUNT_PERFORMANCE;
	string hostName = "";
	bool isCloseThisSocket = true;
	if (getHost(rcvStr, hostName))
	{
		//获得域名-connect,修改对端socket  分离出prot
		size_t index = hostName.find(":");
		string strPort = "";
		if (index != -1)
		{
			strPort = hostName.substr(index+1);
			hostName = hostName.substr(0, index);
		}
		else
		{
			strPort = getConnectPort(rcvStr);
		}

		IPData _ipData = getIpByHostName(hostName);
		if (_ipData._ip == "")
			return !isCloseThisSocket;

		string str_ip = _ipData._ip;
		SOCKET  ConnectSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (ConnectSocket == INVALID_SOCKET) {
			return !isCloseThisSocket;		//使用下个IP创建
		}
		sockaddr_in clientService;
		clientService.sin_family = AF_INET;
		clientService.sin_addr.s_addr = inet_addr(str_ip.c_str());
		if (strPort=="")
			clientService.sin_port = htons(80);
		else
		{
			USHORT port = atoi(strPort.c_str());
			if (port == 443)
			{
				sockPair.httpSocket.rcvData = m_httpsResp;
				sockPair.clientSocket.rcvData.clear();
			}
			clientService.sin_port = htons(port);
		}
		setSocketFIONBIO(ConnectSocket);
		int iResult = connect(ConnectSocket, (SOCKADDR *)& clientService, sizeof(clientService));
		sockPair.httpSocket.socket = ConnectSocket;

		if (SOCKET_ERROR == iResult)
		{
			sockPair.httpSocket.socketStatue = Linking;
			DWORD err = WSAGetLastError();
			if (WSAEWOULDBLOCK == err || WSAEINVAL == err) //无法立即完成非阻塞的操作
			{ }
		}
		isCloseThisSocket = false;
		QString strLog = QString("connect to IP-%1,Port-%2").arg(str_ip.c_str()).arg(clientService.sin_port);
		//printLog(strLog);			
		
	}
	return !isCloseThisSocket;
}

void ProxyServer::ClearSocketPair()
{
	//for (auto ite : m_listSocketPair)
	for (auto ite = m_listSocketPair.begin();ite != m_listSocketPair.end();)
	{
		bool isCl = false;
		socketPair socPair = (*ite);
		int lenCData = socPair.clientSocket.rcvData.size();
		int lenHData = socPair.httpSocket.rcvData.size();

		if (socPair.clientSocket.socketStatue == CLOSED&& (lenHData == 0|| lenCData==0))
			isCl = true;
		else if (socPair.httpSocket.socketStatue == CLOSED&&lenCData == 0)
			isCl = true;
		else if (socPair.httpSocket.socket==0 )
		{
			long long disTime = time(NULL) - socPair.clientSocket.create_time;
			if (disTime>3)
				isCl = true;
		}

		if (isCl)
		{
			closesocket((*ite).clientSocket.socket);
			FD_CLR((*ite).clientSocket.socket, &m_allSet);
			FD_CLR((*ite).clientSocket.socket, &m_writeSet);

			closesocket((*ite).httpSocket.socket);
			FD_CLR((*ite).httpSocket.socket, &m_allSet);
			FD_CLR((*ite).httpSocket.socket, &m_writeSet);
			ite = m_listSocketPair.erase(ite);
			continue;
		}
		ite++;
	}
}

void ProxyServer::recoderRcvLen(int len)
{
	m_rcvCount = m_rcvCount + len;
	if (m_rcvCount > 512 * 1024)
	{
		printLog(QString::number(m_rcvCount), TOTAL);
		m_rcvCount = 0;
	}
}

void ProxyServer::newSocketToHttp(socketPair&ite, string &rcvStr)
{
	if (!connectToHttpServer(ite, rcvStr))
	{
		ite.clientSocket.socketStatue = CLOSED;
		ite.httpSocket.socketStatue = CLOSED;
		ite.clientSocket.rcvData.clear();
		ite.httpSocket.rcvData.clear();
	}
	else
	{
		if (ite.httpSocket.socketStatue == Linking)
		{
			FD_SET(ite.httpSocket.socket, &m_writeSet);
			FD_SET(ite.httpSocket.socket, &m_allSet);
		}
		else if (ite.httpSocket.rcvData.size() > 0 && ite.httpSocket.socketStatue != Linking)
			FD_SET(ite.clientSocket.socket, &m_writeSet);
	}
}

bool ProxyServer::processReadSocket(  fd_set readSet)
{
	COUNT_PERFORMANCE;
	for (u_int i = 0; i < readSet.fd_count; ++i)
	{
		SOCKET socket = readSet.fd_array[i];
		/// 可读性监视，可读性指有连接到来、有数据到来、连接已关闭、重置或终止
		string rcvStr = "";		
		int rcvRel = rcvSocketData(socket, rcvStr);

		if (rcvStr.size() > 0)
		{
			for (auto &ite : m_listSocketPair)
			{
				if (ite.clientSocket.socket == socket)	//当前是client来的socket
				{
					ite.clientSocket.rcvData.append(rcvStr);		
					if (ite.httpSocket.socket == 0)//还没有跟http建立的socket
					{
						newSocketToHttp(ite, rcvStr);
						break;
					}
					if (ite.httpSocket.socket != 0&& ite.clientSocket.rcvData.size()>0 )
						FD_SET(ite.httpSocket.socket, &m_writeSet);	//http的socket加入到可写
					break;
				}
				else if (ite.httpSocket.socket == socket)	//当前是http来的socket
				{
					if (ite.httpSocket.socketStatue == Linking)
					{
						FD_SET(ite.httpSocket.socket, &m_writeSet);
						ite.httpSocket.socketStatue = NONE;
					}
					//获取数据，把对端的socket加入到可写里面
					ite.httpSocket.rcvData.append(rcvStr);
					if (ite.httpSocket.rcvData.size()>0)
						FD_SET(ite.clientSocket.socket, &m_writeSet);	//对应的client的socket加入到可写
					recoderRcvLen(rcvStr.size());					
					break;
				}
			}
		}
		
		if (rcvRel == 0 || rcvRel == -1)	//socket被关 或是 对方reset,设置关闭标记
		{
			struct sockaddr_in sa;
			int len = sizeof(sa);
			if (!getpeername(socket, (struct sockaddr *)&sa, &len) && rcvRel == -1)
			{
				string strIp = inet_ntoa(sa.sin_addr);
				QString strLog = QString("IP:%1,PORT:%2 --rcvRel = %3").arg(strIp.c_str()).arg(ntohs(sa.sin_port)).arg(rcvRel);
				//printLog(strLog);
				printLog(QString("m_listSocketPair.size = %1").arg(m_listSocketPair.size()));
			}

			for (auto &ite : m_listSocketPair)
				if (ite.clientSocket.socket == socket || ite.httpSocket.socket == socket)
				{
					if (rcvRel == -1)
						FD_CLR(socket, &m_writeSet);
					FD_CLR(socket, &m_allSet);
					if (ite.clientSocket.socket == socket)
						ite.clientSocket.socketStatue = CLOSED;
					else if (ite.httpSocket.socket == socket)
						ite.httpSocket.socketStatue = CLOSED;
					break;
				}			
		}			
	}
	return true;
}

bool ProxyServer::processWriteSocket(fd_set writeSet)
{
	COUNT_PERFORMANCE;
	for (u_int i = 0; i < writeSet.fd_count; ++i)
	{
		SOCKET socket = writeSet.fd_array[i];
		for (auto &ite : m_listSocketPair)
		{
			if (ite.clientSocket.socket == socket)
			{
				sendData(socket, ite.httpSocket.rcvData);
				break;
			}				
			if (ite.httpSocket.socket == socket)
			{
				if (ite.httpSocket.socketStatue == Linking)	//说明http的 socket 连接成功
				{
					if ( !httpSocketConnectSuc(ite) )
						continue;					
				}
				sendData(socket, ite.clientSocket.rcvData);
				break;
			}				
		}		
	}
	return true;
}

bool ProxyServer::httpSocketConnectSuc(socketPair&ite)
{
	COUNT_PERFORMANCE;
	int optval = -1;
	int optlen = sizeof(optval);
	int ret = getsockopt(ite.httpSocket.socket, SOL_SOCKET, SO_ERROR, (char*)(&optval), &optlen);
	if (ret != 0 || optval != 0)
	{
		ite.httpSocket.socketStatue = CLOSED;
		ite.clientSocket.socketStatue = CLOSED;
	}
	else
	{
		ite.httpSocket.socketStatue = NONE;
		FD_SET(ite.clientSocket.socket, &m_writeSet);
	}

	if (ite.clientSocket.rcvData.size() == 0)
		return false;
	return true;
}

bool ProxyServer::sendData(UINT_PTR socket,string &strData)
{
	COUNT_PERFORMANCE;
	int strSize = strData.size();
	int sendLen = -1;
	if (strSize > 0)
	{
		sendLen = send(socket, strData.c_str(), strSize, 0);
		if (sendLen > 0)
		{
			if (sendLen == strSize)
				strData.clear();
			else
				strData.erase(0, sendLen);
		}
		else 
		{
			DWORD err = GetLastError();
			printLog(QString("Send Error = %1,  send res = %2").arg(err).arg(sendLen));
		}
	}
	if (strData.size() == 0 || sendLen == -1)
		FD_CLR(socket, &m_writeSet);
	return true;
}

bool ProxyServer::processListenSocket(UINT_PTR socketListen, fd_set &readSet)
{
	if (FD_ISSET(socketListen, &readSet))		//说明是监听socket，所以对他进行accepte
	{
		SOCKET clientSocket = 0;
		if (getAcceptSocket(socketListen, clientSocket))
		{
			setSocketFIONBIO(clientSocket);
			FD_SET(clientSocket, &m_allSet);		// 将新创建的套接字加入到读集合中
			socketPair socket_pair;
			socket_pair.clientSocket.socket = clientSocket;
			socket_pair.clientSocket.create_time = time(NULL);
			m_listSocketPair.append(socket_pair);
			FD_CLR(socketListen, &readSet);
		}
	}
	return true;
}

bool ProxyServer::StartServer()
{
	//使用windows select模型-绑定 监听 转发---纯转发无验证
	QString strLog = "";
	printLog(QString("StartServer"));

	SOCKET socketListen;
	if ( !preSocketInit(socketListen) )
	{
		return false;
	}

	FD_SET(socketListen, &m_allSet); // 将socketListen加入套接字集合中
	m_listenSocket = socketListen;
	setSocketFIONBIO(m_listenSocket);
	fd_set readSet;
	fd_set writedSet;			//写模块另做

	while (true)
	{
		FD_ZERO(&readSet);		
		FD_ZERO(&writedSet);
		readSet = m_allSet;	
		writedSet = m_writeSet;		

		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int result = select(0, &readSet, &writedSet, NULL, &timeout);
		if (result == SOCKET_ERROR)
		{
			DWORD error =  GetLastError();
			printLog(QString("listen() error."));
			break;
		}
		time_t old = time(NULL);
		processListenSocket(m_listenSocket, readSet);
		processReadSocket(readSet);
		processWriteSocket( writedSet);
		ClearSocketPair();
		time_t curt = time(NULL);
		if (curt - old >= 1)
		{
			printLog(QString("********************%1秒").arg(curt - old));
		}
	}

	endSocket();
	return true;
}
