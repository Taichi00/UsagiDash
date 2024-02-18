#pragma once

#include "math/vec.h"
#include <string>
#include <vector>
#include <map>

class MapFileParser
{
public:
	struct Entity
	{
		std::map<std::string, std::string> pairs;
	};

	struct Map
	{
		std::vector<Entity> entities;
	};

	MapFileParser();

	bool Load(const std::wstring& path, Map& map);

	static float ToFloat(const std::string& s);
	static Vec3 ToVec3(const std::string& s);

private:
	bool LoadFile(const std::wstring& path, std::string& buf);

	void StartTokenParsing(char* data);

	bool GetToken();

	bool ParsePair(std::string& value);
	bool ParseBrush();
	bool ParseEntity(Entity& entity);

private:
	char* data_ptr_;
	std::string token_;
};