

#include "ProxyServer.h"


ProxyServer::ProxyServer()
{
	m_listSocketPair.clear();
	FD_ZERO(&m_writeSet);
	FD_ZERO(&m_allSet);
	m_httpsResp = "HTTP/1.1 200 Connection Established\r\n\r\n";
}


ProxyServer::~ProxyServer()
{
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
		WSACleanup();
		printLog(QString("socket() error."));
		return false;
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
		WSACleanup();
		printLog(QString("bind() error."));
		return false;
	}

	/// 开启监听  
	result = listen(socketListen, 5);
	if (result == SOCKET_ERROR)
	{
		closesocket(socketListen);
		WSACleanup();
		printLog(QString("listen() error."));
		return false;
	}
	strLog = QString("bind:%1").arg(ntohs(svrAddress.sin_port));
	printLog(strLog);
	return true;
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
	printLog(strLog);
	return true;
}

bool ProxyServer::endSocket()
{
	for (u_int i = 0; i < m_allSet.fd_count; ++i)
	{
		SOCKET socket = m_allSet.fd_array[i];
		closesocket(socket);
	}

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
	{
		return result;
	}
	if (result > 0)
		rcvStr.append(bufRecv, result);
	int len = rcvStr.size();
	return result;
}

bool ProxyServer::getHost(const string &rcvStr, string &hostName)
{
	hostName = "";
	size_t index1 = rcvStr.find("Host:");
	if (index1 != string::npos)
	{
		int index2 = rcvStr.find("\r\n", index1);
		string subStr = rcvStr.substr(index1, index2 - index1);
		hostName = subStr.substr(6);
		if (subStr == "" || hostName == "")
			return false;
		else
			return true;
	}
	return false;
}

bool ProxyServer::setSocketFIONBIO(UINT_PTR ConnectSocket)
{
	unsigned long ul = 1;
	int res = ioctlsocket(ConnectSocket, FIONBIO, &ul); //设置为非阻塞模式
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

bool ProxyServer::connectToHttpServer(socketPair& sockPair, const string &rcvStr)
{					
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

		hostent *phost = gethostbyname(hostName.c_str());
		if (phost)
		{
			for (int i = 0; phost->h_addr_list[i]; i++)
			{
				char str[32] = { 0 };
				string str_ip = inet_ntop(phost->h_addrtype, phost->h_addr_list[i], str, sizeof(str) - 1);
				SOCKET  ConnectSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if (ConnectSocket == INVALID_SOCKET) {
					continue;		//使用下个IP创建
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
				if (SOCKET_ERROR == iResult)
				{
					DWORD err = WSAGetLastError();
					if (WSAEWOULDBLOCK == err || WSAEINVAL == err) //无法立即完成非阻塞的操作
					{ }
				}				
				QString strLog = QString("connect to IP-%1,Port-%2").arg(str_ip.c_str()).arg(clientService.sin_port);
				printLog(strLog);
				sockPair.httpSocket.socket = ConnectSocket;
				if (sockPair.httpSocket.rcvData.size() > 0)
				{
					FD_SET(sockPair.clientSocket.socket,&m_writeSet);
				}
				FD_SET(ConnectSocket, &m_allSet);
				isCloseThisSocket = false;
				break;
			}
		}
	}

	if (isCloseThisSocket)
	{
		//关闭socket		
		cleanSocketPair(sockPair.clientSocket.socket);
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
		if (socPair.clientSocket.socketStatue == CLOSED && socPair.httpSocket.socketStatue == CLOSED)
			isCl = true;
		
		if ((socPair.clientSocket.socketStatue == CLOSED || socPair.httpSocket.socketStatue == CLOSED) && lenCData==0&& lenHData==0)
			isCl = true;

		if (socPair.clientSocket.socketStatue == CLOSED&&lenHData == 0)
			isCl = true;

// 		if ((*ite).clientSocket.socket != 0 && (*ite).httpSocket.socket!=0)
// 			if (lenCData == 0 && lenHData == 0)
// 				isCl = true;

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

void ProxyServer::cleanSocketPair(UINT_PTR sock)
{
	closesocket(sock);
	FD_CLR(sock, &m_allSet);
	FD_CLR(sock, &m_writeSet);

	for (auto &ite : m_listSocketPair)
	{
		if (ite.clientSocket.socket == sock)
		{
			ite.clientSocket.socketStatue = CLOSED;
		}
		else if(ite.httpSocket.socket == sock)
		{
			ite.httpSocket.socketStatue = CLOSED;
		}
	}
}

bool ProxyServer::processReadSocket(  fd_set readSet)
{
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
						connectToHttpServer(ite, rcvStr);
					if (ite.httpSocket.socket != 0)
						FD_SET(ite.httpSocket.socket, &m_writeSet);	//http的socket加入到可写
					break;
				}
				else if (ite.httpSocket.socket == socket)	//当前是http来的socket
				{
					//获取数据，把对端的socket加入到可写里面
					ite.httpSocket.rcvData.append(rcvStr);
					FD_SET(ite.clientSocket.socket, &m_writeSet);	//对应的client的socket加入到可写
					printLog(QString::number(rcvStr.size()), TOTAL);
					break;
				}
			}
		}
		
		if (rcvRel == 0 || rcvRel == -1)	//socket被关 或是 对方reset,设置关闭标记
		{
			struct sockaddr_in sa;
			int len = sizeof(sa);
			if (!getpeername(socket, (struct sockaddr *)&sa, &len))
			{
				string strIp = inet_ntoa(sa.sin_addr);
				QString strLog = QString("IP:%1,PORT:%2 --rcvRel = %3").arg(strIp.c_str()).arg(ntohs(sa.sin_port)).arg(rcvRel);
				printLog(strLog);
				printLog(QString("m_listSocketPair.size = %1").arg(m_listSocketPair.size()));
			}

			for (auto &ite : m_listSocketPair)
				if (ite.clientSocket.socket == socket || ite.httpSocket.socket == socket)
				{
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

bool ProxyServer::sendData(UINT_PTR socket)
{
	for (auto &ite : m_listSocketPair)
	{
		if (ite.clientSocket.socket == socket)		
		{
			int strSize = ite.httpSocket.rcvData.size();
			if (strSize > 0)
			{
				int sendLen = send(socket, ite.httpSocket.rcvData.c_str(), strSize, 0);
				if (sendLen == strSize)
					ite.httpSocket.rcvData.clear();
				else
					ite.httpSocket.rcvData.erase(0, sendLen);
			}
			if (ite.httpSocket.rcvData.size() == 0)
				FD_CLR(socket, &m_writeSet);			
		}
		else if (ite.httpSocket.socket == socket)
		{
			int strSize = ite.clientSocket.rcvData.size();
			if (strSize > 0)
			{
				int sendLen = send(socket, ite.clientSocket.rcvData.c_str(), strSize, 0);
				if (sendLen == strSize)
					ite.clientSocket.rcvData.clear();
				else
					ite.clientSocket.rcvData.erase(0, sendLen);
			}
			if (ite.clientSocket.rcvData.size() == 0)
				FD_CLR(socket, &m_writeSet);
		}
	}
	return true;
}

bool ProxyServer::processWriteSocket(fd_set writeSet)
{
	for (u_int i = 0; i < writeSet.fd_count; ++i)
	{
		SOCKET socket = writeSet.fd_array[i];
		sendData(socket);
	}
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
	//setSocketFIONBIO(m_listenSocket);
	fd_set readSet;
	fd_set writedSet;			//写模块另做

	while (true)
	{
		FD_ZERO(&readSet);		
		FD_ZERO(&writedSet);
		readSet = m_allSet;	
		writedSet = m_writeSet;		
		int result = select(0, &readSet, &writedSet, NULL, NULL);
		if (result == SOCKET_ERROR)
		{
			DWORD error =  GetLastError();
			printLog(QString("listen() error."));
			break;
		}
		//printLog(QString("m_listSocketPair.size = %1").arg(m_listSocketPair.count()));
		processListenSocket(m_listenSocket, readSet);
		processReadSocket(readSet);
		processWriteSocket( writedSet);
		ClearSocketPair();
	}

	endSocket();
	return true;
}
