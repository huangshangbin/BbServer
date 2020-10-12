#pragma once

#include "BbStringUtils.hpp"

#include <deque>
#include <string>
#include <map>
using namespace std;

class BbRequestUtils
{
public:
	static bool isCompleteData(string& dataBuffer)
	{
		if (dataBuffer[0] == 'G')
		{
			return isCompleteGetRequest(dataBuffer);
		}

		if (dataBuffer[0] == 'P' && dataBuffer[1] == 'O')
		{
			return isCompletePostRequest(dataBuffer);
		}

		return true;
	}


//isCompleteData
private:
	static bool isCompleteGetRequest(string& dataBuffer)
	{
		int length = dataBuffer.length();

		if (dataBuffer[length - 1] == '\n' && dataBuffer[length - 2] == '\r' && dataBuffer[length - 3] == '\n')
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	static bool isCompletePostRequest(string& dataBuffer)
	{
		string body = BbStringUtils::splitStringGetOneStr(dataBuffer, "\r\n\r\n", 1);
		if (body.length() == getContentLength(dataBuffer))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

//isCompletePostRequest
private:
	static long getContentLength(string& requestStream)
	{
		int hCharIndex = 0;
		for (int i = 0; i < requestStream.length(); i++)
		{
			if (requestStream[i] == 'C' && requestStream[i + 1] == 'o' && requestStream[i + 7] == '-'
				&& requestStream[i + 8] == 'L' && requestStream[i + 13] == 'h')
			{
				hCharIndex = i + 13;
				break;
			}
		}

		int lengthStartIndex = 0;
		for (int i = hCharIndex; i < requestStream.length(); i++)
		{
			if (requestStream[i] != ' ' && requestStream[i] != ':')
			{
				lengthStartIndex = i;
				break;
			}
		}

		int legthEndIndex = 0;
		for (int i = lengthStartIndex; i < requestStream.length(); i++)
		{
			if (requestStream[i] == '\r' || requestStream[i] == '\n')
			{
				legthEndIndex = i;
				break;
			}
		}

		string lengthStr = requestStream.substr(lengthStartIndex, legthEndIndex - lengthStartIndex + 1);
		return atol(lengthStr.c_str());
	}
};