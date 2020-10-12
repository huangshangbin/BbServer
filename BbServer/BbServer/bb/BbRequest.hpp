#pragma once


#include <bb/utils/BbStringUtils.hpp>


#include <iostream>
#include <string>

#include <map>


using namespace std;


class BbRequest 
{
public:
	string m_method;
	string m_path;
	string m_version;

	map<string, string> m_headerMap;

	string m_body;


public:
	void parserRequest(string& requestData)
	{
		deque<string> dataList = BbStringUtils::splitString(requestData, "\n");
		if (dataList[0][dataList[0].length() - 1] == '\r')
		{
			dataList = BbStringUtils::splitString(requestData, "\r\n");
		}
		
		m_method = BbStringUtils::getStringUseCharEnd(dataList[0], ' ');
		m_path = "/" + BbStringUtils::getStringUseCharEnd(BbStringUtils::getStringUseCharStart(dataList[0], '/'), ' ');
		m_version = BbStringUtils::splitStringGetOneStr(dataList[0], "/", 2);
		setHeader(dataList);
		setBody(requestData);
	}


//parserRequest
private:
	void setHeader(deque<string>& dataList)
	{
		int bodyIndex = 0;
		for (int i = 1; i < dataList.size(); i++)
		{
			if (dataList[i] == "")
			{
				bodyIndex = i + 1;
				break;
			}

			deque<string> tempDataList = BbStringUtils::splitString(dataList[i], ":");
			m_headerMap[tempDataList[0]] = tempDataList[1];
		}
	}

	void setBody(string& requestData)
	{
		int dataLength = requestData.length();
		int bodyStartIndex = 0;
		for (int i = 0; i < requestData.length(); i++)
		{
			if (requestData[i] == '\r' && requestData[i + 1] == '\n' && requestData[i + 2] == '\r' && requestData[i + 3] == '\n')
			{
				bodyStartIndex = i + 4;
				break;
			}
		}

		m_body = requestData.substr(bodyStartIndex, requestData.length() - bodyStartIndex);
	}
};