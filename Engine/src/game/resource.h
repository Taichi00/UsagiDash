#pragma once
#include <string>

class Resource
{
public:
	Resource();
	~Resource();

	virtual bool Release();

private:
	std::string key_;	// リソースを識別するためのキー（通常はファイルへのパス）
};