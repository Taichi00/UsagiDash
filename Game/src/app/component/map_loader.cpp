#include "map_loader.h"
#include "game/scene.h"
#include "app/entity/coin.h"
#include "app/entity/tutorial.h"
#include "app/entity/checkpoint.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "app/component/game_manager.h"
#include "app/entity/metal_ball_emitter.h"
#include "app/entity/jump_lift.h"
#include "app/entity/star.h"
#include "app/entity/ball.h"

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

	// MapFileのパース
	if (!parser.Load(path_, map)) return false;

	LoadEntities(map);

	return true;
}

void MapLoader::LoadEntities(const MapFileParser::Map& map)
{
	auto scene = GetScene();
	
	auto map_position = transform->position;
	auto map_scale = transform->scale;

	// コインの枚数
	unsigned int coin_num = 0;

	// Entityの読み込み
	for (const auto& entity : map.entities)
	{
		auto& pairs = entity.pairs;
		auto class_name = pairs.at("classname");

		// Brush Entityは無視
		if (class_name == "worldspawn") continue;

		// 位置
		Vec3 position;
		if (pairs.contains("origin"))
		{
			position = MapFileParser::ToVec3(pairs.at("origin"));
		}
		position = Vec3::Scale(position, map_scale) + map_position;

		// 角度
		Quaternion rotation;
		if (pairs.contains("angle"))
		{
			rotation = Quaternion::FromEuler(0, (float)(MapFileParser::ToFloat(pairs.at("angle")) * PI / 180), 0);
		}
		
		// エンティティの生成
		Entity* new_entity = nullptr;

		if (class_name == "info_player_start")
		{
			// スタート位置
			GameManager::Get()->SetStartPosition(position);
		}
		else if (class_name == "item_coin")
		{
			// コイン
			new_entity = new Coin("");
			coin_num++;
		}
		else if (class_name == "tutorial")
		{
			// チュートリアル
			new_entity = new Tutorial(
				pairs.at("text"), 
				pairs.at("task"),
				MapFileParser::ToFloat(pairs.at("radius"))
			);
		}
		else if (class_name == "checkpoint")
		{
			// チェックポイント
			new_entity = new Checkpoint();
		}
		else if (class_name == "metal_ball_emitter")
		{
			// 鉄球発射装置
			new_entity = new MetalBallEmitter(
				MapFileParser::ToFloat(pairs.at("radius")),
				MapFileParser::ToFloat(pairs.at("width")),
				MapFileParser::ToFloat(pairs.at("spawn_rate"))
			);
		}
		else if (class_name == "jump_rift")
		{
			// ジャンプリフト
			new_entity = new JumpLift(
				Vec3(
					MapFileParser::ToFloat(pairs.at("dir_x")),
					MapFileParser::ToFloat(pairs.at("dir_y")),
					MapFileParser::ToFloat(pairs.at("dir_z"))
				),
				MapFileParser::ToFloat(pairs.at("distance"))
			);
		}
		else if (class_name == "ball")
		{
			// ボール
			auto radius = MapFileParser::ToFloat(pairs.at("radius"));

			new_entity = new Ball(
				radius,
				MapFileParser::ToFloat(pairs.at("mass"))
			);
			position.y += radius;
		}
		else if (class_name == "star")
		{
			// スター
			new_entity = new Star();
		}

		if (new_entity)
		{
			new_entity->transform->position = position;
			new_entity->transform->rotation = rotation;
			scene->CreateEntity(new_entity);
		}
	}

	GameManager::Get()->SetCoinMax(coin_num);
}

