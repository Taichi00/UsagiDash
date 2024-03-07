#pragma once

#include "game/entity.h"

class CustomButton : public Entity
{
public:
	CustomButton(const std::string& name);
	~CustomButton() {};
};