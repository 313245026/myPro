#pragma once


#include <iostream>
using namespace std;

#define SAFE_DEL(ptr) if(ptr) { delete ptr; ptr = NULL;}
typedef  unsigned int UINT_PTR;


enum SocketStatue
{
	CLOSED,
	NONE
};

enum LogType
{
	STR_LOG,
	SPEED,
	TOTAL
};

struct socketData
{
	UINT_PTR socket;
	string rcvData;
	SocketStatue socketStatue;
	socketData()
	{
		socket = 0;
		rcvData = "";
		socketStatue = NONE;
	}	
};

struct socketPair
{
	socketData clientSocket;
	socketData httpSocket;
	bool operator ==(const socketPair &other)
	{
		if (clientSocket.socket = other.clientSocket.socket && httpSocket.socket == other.httpSocket.socket)
			return true;
		return false;
	};
};