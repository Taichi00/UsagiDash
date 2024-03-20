#pragma once

#include "game/component/component.h"
#include <memory>

#define COIN_SILVER_RATE 0.7f
#define COIN_GOLD_RATE 1.0f

#define DEATH_SILVER_NUM 10
#define DEATH_GOLD_NUM 0

class Bitmap;

class ResultManager : public Component
{
public:
	struct CrownScores
	{
		int gold_score = 0;
		int silver_score = 0;
	};

	ResultManager(
		CrownScores time
	);
	~ResultManager() {}

	bool Init() override;

private:
	void ShowResult();
	void ShowScores();

	std::shared_ptr<Bitmap> GetCrownBitmap(int rank);

private:
	// ‰¤Š¥Žæ“¾‚Ìè‡’l
	CrownScores time_crown_scores_ = {};
};