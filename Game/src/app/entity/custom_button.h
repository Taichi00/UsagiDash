#pragma once

#include "game/entity.h"
#include <string>
#include <functional>

class CustomButton : public Entity
{
public:
	CustomButton(
		const std::string& name,
		const std::string& text,
		const std::function<void()>& function
	);
	~CustomButton() {};
};