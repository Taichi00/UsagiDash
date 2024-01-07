#pragma once
#include <string>

class Resource
{
public:
	Resource();
	~Resource();

	virtual bool Release();

private:
	std::string m_key;	// リソースを識別するためのキー（通常はファイルへのパス）
};