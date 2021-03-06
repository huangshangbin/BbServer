#pragma once

#include <bb/utils/BbPathUtils.hpp>

class BbService
{
public:
	map<string, BbPathFun> m_getPathFunMap;
	map<string, BbPathFun> m_postPathFunMap;


public:
	void bindGet(string path, BbPathFun pathFun)
	{
		m_getPathFunMap[path] = pathFun;
	}

	void bindPost(string path, BbPathFun pathFun)
	{
		m_postPathFunMap[path] = pathFun;
	}
};
