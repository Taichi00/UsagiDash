#pragma once
#include <string>

class Resource
{
public:
	Resource();
	~Resource();

	virtual bool Release();

	std::wstring Name() const { return name_; }
	void SetName(const std::wstring& name);

protected:
	std::wstring name_;
};