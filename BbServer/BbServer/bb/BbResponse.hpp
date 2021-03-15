#pragma once

#include <bb/utils/BbFile.hpp>


#include <iostream>
#include <string>
#include <map>

using namespace std;

class BbResponse
{
public:
	string m_version = "1.1";
	int m_status = 200;

	map<string, string> m_headerMap = { {"Server", "BuildingBlocks"}, {"Accept-Rangers", "none"}, {"Content-Length", "0"} };

	string m_body;

	map<int, string> m_statusExplainMap = { {200, "OK"}, {400, "Bad Request"}, 
			{500, "Internal Server Error"}, {505, "HTTP Version Not Supported"} };


//for user
public:
	void replyText(string text)
	{
		m_headerMap["Content-Type"] = "text/plain";
		m_headerMap["Content-Length"] = std::to_string(text.size());

		m_body = text;
	}

	void replyJson(string jsonStr)
	{
		m_headerMap["Content-Type"] = "application/json";
		m_headerMap["Content-Length"] = std::to_string(jsonStr.size());

		m_body = jsonStr;
	}

	void replyBinary(string binaryStr)
	{
		m_headerMap["Content-Type"] = "application/octet-stream";
		m_headerMap["Content-Length"] = std::to_string(binaryStr.size());

		m_body = binaryStr;
	}

	void replyHtml(string htmlStr)
	{
		m_headerMap["Content-Type"] = "text/html";
		m_headerMap["Content-Length"] = std::to_string(htmlStr.size());

		m_body = htmlStr;
	}

	void replyFile(string filePath)//No matter what file, the browser will download it directly
	{
		string fileSuffix = BbStringUtils::splitStringGetOneStr(filePath, ".", 1);
		if (fileSuffix == "")
		{
			m_status = 500;

			m_headerMap["Content-Type"] = "text/plain";

			m_body = "server error";
			m_headerMap["Content-Length"] = std::to_string(m_body.size());
		}else
		{
			string fileName = getFileOrDirName(filePath);
			m_headerMap["Content-Disposition"] = "attachment;filename=" + fileName;

			BbFile responseFile(filePath);
			m_headerMap["Content-Length"] = std::to_string(responseFile.getLength());

			m_body.append(responseFile.getBuffer(), responseFile.getLength());
		}
	}

	void replyPathNotExist()
	{
		m_status = 400;

		m_headerMap["Content-Type"] = "text/plain";

		m_body = "route error";
		m_headerMap["Content-Length"] = std::to_string(m_body.size());
	}

	void replyNotSupportedMethond()
	{
		string promt = "request method not supported!";

		m_headerMap["Content-Type"] = "text/plain";
		m_headerMap["Content-Length"] = std::to_string(promt.size());

		m_body = promt;
	}


//for bb server
public:
	void generateResponse(string& responseStream)
	{
		responseStream = "HTTP/" + m_version + " " + std::to_string(m_status) + " " + m_statusExplainMap[m_status] + "\r\n";
		
		for (auto& pair : m_headerMap)
		{
			responseStream = responseStream + pair.first + ":" + pair.second + "\r\n";
		}

		responseStream = responseStream + "\r\n";
		responseStream = responseStream + m_body;
	}

//replyFile
private:
	string getFileOrDirName(string path)
	{
		int index;
		for (int i = path.length(); i > 0; i--)
		{
			if (i == 0)
			{
				return path;
			}

			if (path[i] == '\\')
			{
				index = i;
				break;
			}
		}

		string result = BbStringUtils::getStringUsePos(path, index + 1, path.length());
		return result;
	}
};