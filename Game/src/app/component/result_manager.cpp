#include "result_manager.h"
#include "game/scene.h"
#include "game/component/gui/label.h"
#include "game/component/gui/picture.h"
#include "app/component/bgm_manager.h"
#include "game/entity.h"
#include "game/component/timeline_player.h"
#include "app/component/game_manager.h"
#include <string>

ResultManager::ResultManager(CrownScores time)
{
	time_crown_scores_ = time;
}

bool ResultManager::Init()
{
	ShowResult();

	return true;
}

void ResultManager::ShowResult()
{
	// スコアの表示
	ShowScores();

	// タイムライン再生
	GetEntity()->GetComponent<TimelinePlayer>()->Play();

	// BGM再生
	BGMManager::Get()->Play(Game::Get()->LoadResource<Audio>(L"assets/bgm/You_re in the Future.wav"), 0.4f);
	BGMManager::Get()->SetVolume(1, 120);
}

void ResultManager::ShowScores()
{
	auto background_panel = GetEntity()->Child("background_panel");

	auto time = GameManager::Get()->Time();
	auto coin = GameManager::Get()->CoinCount();
	auto coin_max = GameManager::Get()->CoinMax();
	auto death = GameManager::Get()->DeathCount();

	auto time_label = background_panel->Child("time_score")->GetComponent<Label>();
	auto coin_label = background_panel->Child("coin_score")->GetComponent<Label>();
	auto death_label = background_panel->Child("death_score")->GetComponent<Label>();

	auto time_crown = background_panel->Child("time_crown")->GetComponent<Picture>();
	auto coin_crown = background_panel->Child("coin_crown")->GetComponent<Picture>();
	auto death_crown = background_panel->Child("death_crown")->GetComponent<Picture>();

	// スコアの算出
	int min = (int)(time / 60);
	int sec = (int)time % 60;
	int ms = (int)(time * 100) % 100;
	int time_sec = min * 60 + sec;

	int time_rank = 
		(time_sec < time_crown_scores_.gold_score) + 
		(time_sec < time_crown_scores_.silver_score);

	int coin_rank = 
		(coin >= coin_max * COIN_GOLD_RATE) + 
		(coin >= coin_max * COIN_SILVER_RATE);

	int death_rank = 
		(death <= DEATH_GOLD_NUM) + 
		(death <= DEATH_SILVER_NUM);

	// ラベルの設定
	auto min_str = std::to_string(min);
	auto sec_str = std::to_string(sec);
	auto ms_str = std::to_string(ms);

	auto time_str =
		std::string(std::max(0, 2 - (int)min_str.size()), '0') + min_str + ":" +
		std::string(std::max(0, 2 - (int)sec_str.size()), '0') + sec_str + ":" +
		std::string(std::max(0, 2 - (int)ms_str.size()), '0') + ms_str;

	auto coin_str = std::to_string(coin) + "/" + std::to_string(coin_max);
	auto death_str = std::to_string(death);

	time_label->SetText(time_str);
	coin_label->SetText(coin_str);
	death_label->SetText(death_str);

	// 王冠
	time_crown->Load(GetCrownBitmap(time_rank));
	coin_crown->Load(GetCrownBitmap(coin_rank));
	death_crown->Load(GetCrownBitmap(death_rank));
}

std::shared_ptr<Bitmap> ResultManager::GetCrownBitmap(int rank)
{
	std::shared_ptr<Bitmap> bitmap = nullptr;

	switch (rank)
	{
	case 2:
		bitmap = Game::Get()->LoadResource<Bitmap>(L"assets/image/crown_gold.png");
		break;
	case 1:
		bitmap = Game::Get()->LoadResource<Bitmap>(L"assets/image/crown_silver.png");
		break;
	case 0:
		bitmap = Game::Get()->LoadResource<Bitmap>(L"assets/image/crown_bronze.png");
		break;
	}

	return bitmap;
}
