#pragma once

#include <deque>
#include <string>
#include <windows.h>

#include <iostream>
using namespace std;


class BbStringUtils
{
public:
	static deque<string> splitString(string srcStr, string splitStr)
	{
		deque<string> dataList;
		if (srcStr.length() == 0)
		{
			return std::move(dataList);
		}

		string str = srcStr;

		string::size_type pos1, pos2;
		pos2 = str.find(splitStr);
		pos1 = 0;
		while (string::npos != pos2)
		{
			dataList.push_back(str.substr(pos1, pos2 - pos1));

			pos1 = pos2 + splitStr.size();
			pos2 = str.find(splitStr, pos1);
		}
		dataList.push_back(str.substr(pos1));

		return std::move(dataList);
	}

	static string splitStringGetOneStr(string srcStr, string splitStr, int index)
	{
		deque<string> strList = splitString(srcStr, splitStr);
		if (index >= strList.size() || index < 0)
		{
			return "";
		}

		return strList[index];
	}

	static string getStringUsePos(string src, int startPos, int endPos)
	{
		return src.substr(startPos, endPos - startPos + 1);
	}

	static string getStringUseCharStart(string src, char startChar)
	{
		int index = -1;
		for (int i = 0; i < src.length(); i++)
		{
			if (src[i] == startChar)
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			return "";
		}

		return getStringUsePos(src, index + 1, src.length());
	}

	static string getStringUseCharEnd(string src, char endChar)
	{
		int index = -1;
		for (int i = 0; i < src.length(); i++)
		{
			if (src[i] == endChar)
			{
				index = i;
				break;
			}
		}
		if (index == -1)
		{
			return "";
		}

		return getStringUsePos(src, 0, index - 1);
	}
};

