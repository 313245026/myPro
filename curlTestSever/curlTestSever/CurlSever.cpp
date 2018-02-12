#include "CurlSever.h"



#include <string>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include <QString>
#include <QDateTime>



#define USE_MULTI_CURL
struct timeval begin_tv, end_tv;
#define MULTI_CURL_NUM 3

std::string     URL = "http://www.baidu.com"; //"http://website.com";		// 这里设置你需要访问的url // 
std::string     PROXY = "";//"ip:port";										 // 这里设置代理ip和端口  //
unsigned int    TIMEOUT = 5000; /* ms */// 这里设置超时时间  //


CurlSever::CurlSever(QObject *parent)
{
	// 初始化一个multi curl 对象 //
	if (m_pMultiHandle == nullptr)
	{
		CURLM * curl_m = curl_multi_init();
		m_pMultiHandle = curl_m;
	}
	m_listCurl.clear();
	m_mapCurlStr.clear();
	m_mutex = new QMutex;
	m_mutex2 = new QMutex;
}

CurlSever::~CurlSever()
{
	if (m_pMultiHandle)
	{
		curl_multi_cleanup(m_pMultiHandle);
		m_pMultiHandle = nullptr;
	}
	
	for (auto itm : m_listCurl)	//清理 *curl
	{
	}
}

size_t curl_writer(void *buffer, size_t size, size_t count, void * stream)
{
 	std::string * pStream = static_cast<std::string *>(stream);
 	(*pStream).append((char *)buffer, size * count);
	return size * count;
};

intptr_t CurlSever::Post(const std::string & sUrl, const std::string data, const std::string & sProxy, unsigned int uiTimeout)
{
	return 0;
}
intptr_t CurlSever::Get(const std::string & sUrl, const std::string & sProxy, unsigned int uiTimeout)
{
	CURL *_curl = curl_easy_handler(sUrl,sProxy,uiTimeout);
	m_mutex->lock();
	m_listCurl.append(_curl);
	curl_multi_add_handle(m_pMultiHandle, _curl);
	m_waitCondition.wakeAll();	//唤醒线程
	m_mutex->unlock();	
	
	return (intptr_t)_curl;
}

intptr_t CurlSever::Gets(std::string & strUrl, const std::string & sProxy, char* szHost, const char * pCaPath)
{
	CURL * curl = curl_easy_init();
	int uiTimeout = 20000;
	if (uiTimeout > 0)
	{
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, uiTimeout);
	}
	if (!sProxy.empty())
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, sProxy.c_str());
	}

	curl_easy_setopt(curl, CURLOPT_URL, strUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);

	std::string *str = new std::string;
	m_mapCurlStr.insert((intptr_t)curl, str);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

// 	curl_easy_setopt(curl, CURLOPT_HEADER, 1);
// 	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	if (NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}

	struct curl_slist *headers = NULL;
	if (NULL != szHost)
	{
		headers = curl_slist_append(headers, szHost);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	}


	m_mutex->lock();
	m_listCurl.append(curl);
	curl_multi_add_handle(m_pMultiHandle, curl);
	m_waitCondition.wakeAll();	//唤醒线程
	m_mutex->unlock();

	return (intptr_t)curl;
}

//生成一个easy curl对象，进行一些简单的设置操作
CURL * CurlSever::curl_easy_handler(const std::string & sUrl,
	const std::string & sProxy,
	unsigned int uiTimeout)
{
	CURL * curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, sUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if (uiTimeout > 0)
	{
		curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, uiTimeout);
	}
	if (!sProxy.empty())
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, sProxy.c_str());
	}
	// write function //
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);
	std::string *str = new std::string;
	m_mapCurlStr.insert((intptr_t)curl, str);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);
	return curl;
}

int CurlSever::curl_multi_demo()
{
	
	if (m_listCurl.count() == 0)
	{
		m_mutex2->lock();
		m_waitCondition.wait(m_mutex2);
		m_mutex2->unlock();
	}	
	CURLM * curl_m = m_pMultiHandle;
	/*
	* 调用curl_multi_perform函数执行curl请求
	* url_multi_perform返回CURLM_CALL_MULTI_PERFORM时，表示需要继续调用该函数直到返回值不是CURLM_CALL_MULTI_PERFORM为止
	* running_handles变量返回正在处理的easy curl数量，running_handles为0表示当前没有正在执行的curl请求
	*/
	m_mutex->lock();
	int running_handles;
	(CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles));
	{
		cout << running_handles << endl;
	}

	// 为了避免循环调用curl_multi_perform产生的cpu持续占用的问题，采用select来监听文件描述符
	//while (running_handles) 
	{
		int vl_Ret = MultiSelect();
		if (-2 == vl_Ret)
		{
			// curl_multi_fdset返回没有fd可以select  
			/*while*/ (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(m_pMultiHandle, &running_handles));
			{
				int i = 10;
			}
		}
		else if (-1 == vl_Ret)
		{
			//S_TRACE("select error\n");
			//break;
		}
		else
		{
			// select监听到事件，调用curl_multi_perform通知curl执行相应的操作 //  
			/*while*/ (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(m_pMultiHandle, &running_handles));
			{	
				int i = 10;
			}
		}
		// 输出执行结果 //
		int         msgs_left;
		CURLMsg *   msg =  curl_multi_info_read(curl_m, &msgs_left);
		if (msg)
		{
			bool hasMsg = false;
			if (CURLMSG_DONE == msg->msg)
			{
				int idx = 0;
				for (; idx < m_listCurl.size(); idx++)
				{
					if (msg->easy_handle == m_listCurl[idx])
					{
						hasMsg = true;
						break;
					}
				}

				if (!hasMsg)
				{
					cerr << "curl not found" << endl;
				}
				else
				{
					QString Qstr = "";
					std::string *Stdstr = m_mapCurlStr.take((intptr_t)msg->easy_handle);
						
					Qstr = QString("code = %1,lefCount = %2").arg(msg->data.result).arg(m_listCurl.count());
					if (msg->data.result != CURLE_OK)
					{
						Qstr = Qstr + "***********************************";
					}
// 					emit signal_dispStr(Qstr, (intptr_t)msg->easy_handle);
// 					emit signal_emitData((intptr_t)Stdstr, (intptr_t)msg->easy_handle);
				
					m_listCurl.takeAt(idx);
					curl_multi_remove_handle(curl_m, msg->easy_handle);
					curl_easy_cleanup(msg->easy_handle);
					if (m_listCurl.size() == 0)
					{
						int i = 0;
					}
				}
			}
		}
		else
		{
// 			int i = msgs_left;
// 			QString Qstr = QString("*****lefCountMSG**** = %1").arg(msgs_left);
// 			emit signal_dispStr(Qstr, 0);
		}
	}
	m_mutex->unlock();
	return 0;
}

int CurlSever::MultiSelect(void)
{
	//wait for m_listCurl.count		//wait for Event m_listCurl.count
	int ret = 0;
	struct timeval timeout_tv;
	fd_set  fd_read;
	fd_set  fd_write;
	fd_set  fd_except;
	int     max_fd = -1;

	// 注意这里一定要清空fdset,curl_multi_fdset不会执行fdset的清空操作  //  
	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_ZERO(&fd_except);

	// 设置select超时时间  //  
	timeout_tv.tv_sec = 0;
	timeout_tv.tv_usec = 5000;

	long curl_timeo = -1;
	ret = curl_multi_timeout(m_pMultiHandle, &curl_timeo); // curl_timeo 单位是毫秒  
	if (curl_timeo >= 0) {
		timeout_tv.tv_sec = curl_timeo / 1000/1000;
		if (timeout_tv.tv_sec > 1)
			timeout_tv.tv_usec = curl_timeo - timeout_tv.tv_sec * 1000 * 1000;
		else
			timeout_tv.tv_usec = curl_timeo;
	}
	// 获取multi curl需要监听的文件描述符集合 fd_set
	ret = curl_multi_fdset(m_pMultiHandle, &fd_read, &fd_write, &fd_except, &max_fd);

	/**
	* When max_fd returns with -1,
	* you need to wait a while and then proceed and call curl_multi_perform anyway.
	* How long to wait? I would suggest 100 milliseconds at least,
	* but you may want to test it out in your own particular conditions to find a suitable value.
	*/
	if (-1 == max_fd) {
		return -2;
	}

	/**
	* 执行监听，当文件描述符状态发生改变的时候返回
	* 返回0，程序调用curl_multi_perform通知curl执行相应操作
	* 返回-1，表示select错误
	* 注意：即使select超时也需要返回0，具体可以去官网看文档说明
	*/
	//S_TRACE("Try Select\n");
	int ret_code = ::select(max_fd + 1, &fd_read, &fd_write, &fd_except, &timeout_tv);
	switch (ret_code) {
	case -1:
		/* select error */
		ret = -1;
		break;
	case 0:
		/* select timeout */
	default:
		/* one or more of curl's file descriptors say there's data to read or write*/
		ret = 0;
		break;
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**
* easy curl使用demo
*/
int CurlSever::curl_easy_demo(int num)
{
// 	std::string     RspArray[500];
// 	std::string PROXY = "";
// 	for (int idx = 0; idx < num; ++idx)
// 	{
// 		CURL * curl = curl_easy_handler(URL, PROXY, RspArray[idx], TIMEOUT);
// 		CURLcode code = curl_easy_perform(curl);
// 		cout << "curl [" << idx << "] completed with status: "
// 			<< code << endl;
// 		cout << "rsp: " << RspArray[idx] << endl;
// 
// 		// clear handle //
// 		curl_easy_cleanup(curl);
// 	}

	return 0;
}
/**
* 使用select函数监听multi curl文件描述符的状态
* 监听成功返回0，监听失败返回-1
*/
int CurlSever::curl_multi_select(CURLM * curl_m)
{
	int ret = 0;

	struct timeval timeout_tv;
	fd_set  fd_read;
	fd_set  fd_write;
	fd_set  fd_except;
	int     max_fd = -1;

	// 注意这里一定要清空fdset,curl_multi_fdset不会执行fdset的清空操作  //
	FD_ZERO(&fd_read);
	FD_ZERO(&fd_write);
	FD_ZERO(&fd_except);

	// 设置select超时时间  //
	timeout_tv.tv_sec = 1;
	timeout_tv.tv_usec = 0;

	// 获取multi curl需要监听的文件描述符集合 fd_set //
	while (1)
	{
		curl_multi_fdset(curl_m, &fd_read, &fd_write, &fd_except, &max_fd);
		Sleep(50);
		if (max_fd != -1)
			break;
	}


	/**
	* When max_fd returns with -1,
	* you need to wait a while and then proceed and call curl_multi_perform anyway.
	* How long to wait? I would suggest 100 milliseconds at least,
	* but you may want to test it out in your own particular conditions to find a suitable value.
	*/
	if (-1 == max_fd)
	{
		return -1;
	}

	/**
	* 执行监听，当文件描述符状态发生改变的时候返回
	* 返回0，程序调用curl_multi_perform通知curl执行相应操作
	* 返回-1，表示select错误
	* 注意：即使select超时也需要返回0，具体可以去官网看文档说明
	*/
	int ret_code = ::select(max_fd + 1, &fd_read, &fd_write, &fd_except, &timeout_tv);
	switch (ret_code)
	{
	case -1:
		/* select error */
		ret = -1;
		break;
	case 0:
		/* select timeout */
	default:
		/* one or more of curl's file descriptors say there's data to read or write*/
		ret = 0;
		break;
	}

	return ret;
}

//multi curl使用demo
// int CurlSever::curl_multi_demo()
// {
// 	CURLM * curl_m = m_pMultiHandle;
// 
// 	int num = 10;
// 	std::string     *RspArray = new std::string[num];
// 	CURL *          CurlArray[10];
// 	// 设置easy curl对象并添加到multi curl对象中  //
// 	for (int idx = 0; idx < num; ++idx)
// 	{
// 		CurlArray[idx] = NULL;
// 		CurlArray[idx] = curl_easy_handler(URL, PROXY, RspArray[idx], TIMEOUT);
// 		if (CurlArray[idx] == NULL)
// 		{
// 			return -1;
// 		}
// 		curl_multi_add_handle(curl_m, CurlArray[idx]);
// 	}
// 
// 	/*
// 	* 调用curl_multi_perform函数执行curl请求
// 	* url_multi_perform返回CURLM_CALL_MULTI_PERFORM时，表示需要继续调用该函数直到返回值不是CURLM_CALL_MULTI_PERFORM为止
// 	* running_handles变量返回正在处理的easy curl数量，running_handles为0表示当前没有正在执行的curl请求
// 	*/
// 	int running_handles;
// 	while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(curl_m, &running_handles))
// 	{
// 		cout << running_handles << endl;
// 	}
// 	// 为了避免循环调用curl_multi_perform产生的cpu持续占用的问题，采用select来监听文件描述符
// 	while (running_handles) {
// 		int vl_Ret = MultiSelect();
// 		if (-2 == vl_Ret)
// 		{
// 			// curl_multi_fdset返回没有fd可以select  
// 			while (CURLM_CALL_MULTI_PERFORM	== curl_multi_perform(m_pMultiHandle, &running_handles)) 
// 			{
// 				QString str = QString(",vlRet = %1,running_handles = %2").arg(vl_Ret).arg(running_handles);
// 				str = "time = " + QDateTime::currentDateTime().toString("h:m:s:z") + str;
// 				emit signal_dispStr(str);
// 				//S_TRACE("Running Handles: %d\n", vl_RunningHandles);
// 			}
// 		}
// 		else if (-1 == vl_Ret)
// 		{
// 			//S_TRACE("select error\n");
// 			break;
// 		}
// 		else
// 		{
// 			// select监听到事件，调用curl_multi_perform通知curl执行相应的操作 //  
// 			while (CURLM_CALL_MULTI_PERFORM	== curl_multi_perform(m_pMultiHandle, &running_handles)) 
// 			{
// 				QString str = QString(",vlRet = %1,running_handles = %2").arg(vl_Ret).arg(running_handles);
// 				str = "time = " + QDateTime::currentDateTime().toString("h:m:s:z") + str;
// 				emit signal_dispStr(str);
// 			}
// 			
// 		}
// 	}
// 	// 输出执行结果 //
// 	int         msgs_left;
// 	CURLMsg *   msg;
// 	while ((msg = curl_multi_info_read(curl_m, &msgs_left)))
// 	{
// 		if (CURLMSG_DONE == msg->msg)
// 		{
// 			int idx;
// 			for (idx = 0; idx < num; ++idx)
// 			{
// 				if (msg->easy_handle == CurlArray[idx]) break;
// 			}
// 
// 			if (idx == num)
// 			{
// 				cerr << "curl not found" << endl;
// 			}
// 			else
// 			{
// 				QString Qstr = "";
// 				Qstr = QString("curl [ %1 ] completed with status: %2,RSP=%3").arg(idx).arg(msg->data.result).arg(RspArray[idx].data());
// 				emit signal_dispStr(Qstr);				
// 			}
// 		}
// 	}
// 	// 这里要注意cleanup的顺序 //这里加锁 del m_listCurl
// 	for (int idx = 0; idx < num; ++idx)
// 	{
// 		curl_multi_remove_handle(curl_m, CurlArray[idx]);
// 	}
// 
// 	for (int idx = 0; idx < num; ++idx)
// 	{
// 		curl_easy_cleanup(CurlArray[idx]);
// 	}
// 	return 0;
// }