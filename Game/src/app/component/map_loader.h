#pragma once

#include "game/component/component.h"
#include "app/util/map_file_parser.h"
#include <string>
#include <memory>
#include <map>

class MapLoader : public Component
{
public:
	enum ValueType
	{
		STRING = 0,
		FLOAT,
		VECTOR3,
	};

	MapLoader(const std::wstring& path);

	bool Init() override;

private:
	void LoadEntities(const MapFileParser::Map& map);

private:
	std::wstring path_;

	std::map<std::string, ValueType> value_type_map_;
};