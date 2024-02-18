#include "map_loader.h"
#include "game/scene.h"
#include "app/entity/coin.h"

MapLoader::MapLoader(const std::wstring& path)
{
	path_ = path;

	value_type_map_ =
	{
		{ "classname", ValueType::STRING },
		{ "origin", ValueType::VECTOR3 },
		{ "angle", ValueType::FLOAT },
	};
}

bool MapLoader::Init()
{
	auto parser = MapFileParser();
	MapFileParser::Map map;

	// MapFile‚Ìƒp[ƒX
	if (!parser.Load(path_, map)) return false;

	LoadEntities(map);

	return true;
}

void MapLoader::LoadEntities(const MapFileParser::Map& map)
{
	auto scene = GetScene();
	
	auto position = transform->position;
	auto scale = transform->scale;

	// Entity‚Ì“Ç‚Ýž‚Ý
	for (const auto& entity : map.entities)
	{
		auto& pairs = entity.pairs;
		auto class_name = pairs.at("classname");

		// Brush Entity‚Í–³Ž‹
		if (class_name == "worldspawn") continue;

		auto origin = MapFileParser::ToVec3(pairs.at("origin"));
		origin = Vec3::Scale(origin, scale) + position;

		Entity* new_entity = nullptr;
		if (class_name == "item_coin")
		{
			new_entity = scene->CreateEntity(new Coin(""));
		}

		if (new_entity)
		{
			new_entity->transform->position = origin;
		}
	}
}

