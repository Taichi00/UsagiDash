#pragma once

#include "game/component/component.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include <xaudio2.h>
#include <x3daudio.h>

class AudioListener : public Component
{
public:
	AudioListener();
	~AudioListener();

	void Update(const float delta_time) override;

private:
	X3DAUDIO_LISTENER listener_ = {};

	Vec3 prev_position_;
};