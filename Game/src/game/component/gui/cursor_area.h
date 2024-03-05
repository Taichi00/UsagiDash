#pragma once

#include "game/component/gui/control.h"

class CursorArea : public Control
{
public:
	CursorArea();
	~CursorArea();

	bool Init() override;
	void Update(const float delta_time) override;
	void Draw2D() override;

private:

};