#include "map_loader.h"
#include "game/scene.h"
#include "app/entity/coin.h"
#include "app/entity/tutorial.h"
#include "app/entity/checkpoint.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "app/component/game_manager.h"

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

	// MapFile�̃p�[�X
	if (!parser.Load(path_, map)) return false;

	LoadEntities(map);

	return true;
}

void MapLoader::LoadEntities(const MapFileParser::Map& map)
{
	auto scene = GetScene();
	
	auto map_position = transform->position;
	auto map_scale = transform->scale;

	// Entity�̓ǂݍ���
	for (const auto& entity : map.entities)
	{
		auto& pairs = entity.pairs;
		auto class_name = pairs.at("classname");

		// Brush Entity�͖���
		if (class_name == "worldspawn") continue;

		// �ʒu
		Vec3 position;
		if (pairs.contains("origin"))
		{
			position = MapFileParser::ToVec3(pairs.at("origin"));
		}
		position = Vec3::Scale(position, map_scale) + map_position;

		// �p�x
		Quaternion rotation;
		if (pairs.contains("angle"))
		{
			rotation = Quaternion::FromEuler(0, (float)(MapFileParser::ToFloat(pairs.at("angle")) * PI / 180), 0);
		}
		
		// �G���e�B�e�B�̐���
		Entity* new_entity = nullptr;
		if (class_name == "info_player_start")
		{
			// �X�^�[�g�ʒu��ݒ�
			GameManager::Get()->SetStartPosition(position);
		}
		else if (class_name == "item_coin")
		{
			new_entity = new Coin("");
		}
		else if (class_name == "tutorial")
		{
			new_entity = new Tutorial(
				pairs.at("text"), 
				MapFileParser::ToFloat(pairs.at("radius"))
			);
		}
		else if (class_name == "checkpoint")
		{
			new_entity = new Checkpoint();
		}

		if (new_entity)
		{
			scene->CreateEntity(new_entity);
			new_entity->transform->position = position;
			new_entity->transform->rotation = rotation;
		}
	}
}

