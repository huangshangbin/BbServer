#pragma once

#include <atomic>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>


#include "BbService.hpp"
#include "./utils/BbRequestUtils.hpp"


#define BB_BUFFER_SIZE 1024

struct BbSocketData
{
	WSAOVERLAPPED	m_overlapped;
	WSABUF			m_wsaBuffer;   //异步收发消息的buffer,只是一个地址，需要绑定自己的buffer
	SOCKET			m_clientSocket;

	char			m_buffer[BB_BUFFER_SIZE];
	string m_totalBuffer;
};

class BbServer
{
private:
	atomic_bool m_isStop;

	HANDLE m_iocpHandle;
	deque<thread> m_workThreadList;

private:
	int m_mostThreadCount;

	map<string, BbPathFun> m_getPathFunMap;
	map<string, BbPathFun> m_postPathFunMap;

public:
	BbServer()
	{
		m_isStop = false;
		m_iocpHandle = INVALID_HANDLE_VALUE;

		m_mostThreadCount = std::thread::hardware_concurrency();
	}

	~BbServer()
	{
		m_isStop = true;
		for (thread& workThread : m_workThreadList)
		{
			workThread.join();
		}
	}


public:
	void get(string route, BbPathFun routeFun)
	{
		m_getPathFunMap[route] = routeFun;
	}

	void post(string route, BbPathFun routeFun)
	{
		m_postPathFunMap[route] = routeFun;
	}

	void injectService(BbService* service)
	{
		for (auto& pair : service->m_getPathFunMap)
		{
			m_getPathFunMap[pair.first] = pair.second;
		}

		for (auto& pair : service->m_postPathFunMap)
		{
			m_postPathFunMap[pair.first] = pair.second;
		}

		delete service;
	}


public:
	void setMostThreadCount(int threadCount) { m_mostThreadCount = threadCount;  }
	bool listen(string ip, int port)
	{
		createWorkerThread();

		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		SOCKET serverSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

		SOCKADDR_IN serverSocketAddr;
		serverSocketAddr.sin_family = PF_INET;
		serverSocketAddr.sin_port = htons(port);
		serverSocketAddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

		if (::bind(serverSocket, (struct sockaddr*)&serverSocketAddr, sizeof(SOCKADDR_IN)))
		{
			return false;
		}
		if (::listen(serverSocket, 5))
		{
			return false;
		}

		m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);


		SOCKADDR_IN clientScoketAddr;
		int addrLength = sizeof(SOCKADDR_IN);

		SOCKET clientSocket;
		DWORD receiveByteSize;
		DWORD flags = 0;

		while (true)
		{
			clientSocket = WSAAccept(serverSocket, (struct sockaddr*)&clientScoketAddr, &addrLength, NULL, NULL);

			BbSocketData* bbSocketData = new BbSocketData();
			bbSocketData->m_clientSocket = clientSocket;
			bbSocketData->m_wsaBuffer.len = BB_BUFFER_SIZE;
			bbSocketData->m_wsaBuffer.buf = bbSocketData->m_buffer;

			bbSocketData->m_totalBuffer.clear();

			m_iocpHandle = CreateIoCompletionPort((HANDLE)clientSocket, m_iocpHandle, (DWORD)bbSocketData, 0);


			WSARecv(clientSocket, &(bbSocketData->m_wsaBuffer), 1, &receiveByteSize, &flags,
				&(bbSocketData->m_overlapped), NULL);
		}

		WSACleanup();

		return true;
	}

//listen
private:
	void createWorkerThread()
	{
		for (int i = 0; i < m_mostThreadCount; i++)
		{
			m_workThreadList.emplace_back([this]() {

				DWORD sendByteSize = 0;
				DWORD dwFlags = 0;
				DWORD receiveByteSize = 0;
				BbSocketData* bbSocketData;
				BbSocketData* bbIocpKey;

				while (true)
				{
					if (this->m_isStop)
					{
						break;
					}
					if (this->m_iocpHandle == INVALID_HANDLE_VALUE)
					{
						continue;
					}

					BOOL isSuccess = GetQueuedCompletionStatus(this->m_iocpHandle, &receiveByteSize,
						(PULONG_PTR)&bbIocpKey, (LPOVERLAPPED*)&bbSocketData, INFINITE);

					if (isSuccess == FALSE)//The client disconnected the link
					{
						closesocket(bbSocketData->m_clientSocket);
						delete bbSocketData;
						continue;
					}

					if (receiveByteSize == 0)//Maybe the client sent an empty packet
					{
						closesocket(bbSocketData->m_clientSocket);
						delete bbSocketData;
						continue;
					}

					bbSocketData->m_totalBuffer.append(bbSocketData->m_wsaBuffer.buf, receiveByteSize);
					if ((receiveByteSize < BB_BUFFER_SIZE) || BbRequestUtils::isCompleteData(bbSocketData->m_totalBuffer))
					{
						string response;
						this->handle(bbSocketData->m_totalBuffer, response);
						bbSocketData->m_totalBuffer.clear();

						::send(bbSocketData->m_clientSocket, response.c_str(), response.length(), 0);//Synchronous transmission to avoid data copy
					}

					ZeroMemory(&(bbSocketData->m_overlapped), sizeof(OVERLAPPED));
					ZeroMemory(bbSocketData->m_wsaBuffer.buf, BB_BUFFER_SIZE);
					bbSocketData->m_wsaBuffer.len = BB_BUFFER_SIZE;

					dwFlags = 0;
					WSARecv(bbSocketData->m_clientSocket, &(bbSocketData->m_wsaBuffer), 1, &receiveByteSize, &dwFlags,
						(LPWSAOVERLAPPED)&(bbSocketData->m_overlapped), NULL);
				}
			});
		}
	}

//createWorkerThread
private:
	void handle(string& request, string& response)
	{
		BbRequest bbRequest;
		bbRequest.parserRequest(request);

		BbResponse bbResponse;
		bbPathFunDeal(bbRequest, bbResponse);
		
		bbResponse.generateResponse(response);
	}
	
//handle
private:
	void bbPathFunDeal(BbRequest& bbRequest, BbResponse& bbResponse)
	{
		if (bbRequest.m_method == "GET")
		{
			string bindPath = BbPathUtils::getBindPath(m_getPathFunMap, bbRequest.m_path);
			if (bindPath == "")//path does not exist
			{
				bbResponse.replyPathNotExist();
			}
			else
			{
				bbRequest.m_pathParam = BbPathUtils::getParam(bbRequest.m_path, bindPath);
				m_getPathFunMap[bindPath](bbRequest, bbResponse);
			}
		}else if (bbRequest.m_method == "POST")
		{
			string bindPath = BbPathUtils::getBindPath(m_postPathFunMap, bbRequest.m_path);
			if (bindPath == "")//path does not exist
			{
				bbResponse.replyPathNotExist();
			}
			else
			{
				bbRequest.m_pathParam = BbPathUtils::getParam(bbRequest.m_path, bindPath);
				m_postPathFunMap[bindPath](bbRequest, bbResponse);
			}
		}
		else
		{
			bbResponse.replyNotSupportedMethond();
		}
	}
};


