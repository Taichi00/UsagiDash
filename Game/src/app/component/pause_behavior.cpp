#include "pause_behavior.h"
#include "pause_manager.h"

PauseBehavior::~PauseBehavior()
{
}

bool PauseBehavior::Init()
{
	PauseManager::Get()->Add(GetEntity());
	return true;
}

void PauseBehavior::OnDestroy()
{
	PauseManager::Get()->Remove(GetEntity());
}
