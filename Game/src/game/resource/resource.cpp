#include "game/resource/resource.h"
#include "game/game.h"
#include "game/resource_manager.h"

Resource::Resource()
{
	name_ = L"";
}

Resource::~Resource()
{
}

bool Resource::Release()
{
	return false;
}

void Resource::SetName(const std::wstring& name)
{
	name_ = name;

	Game::Get()->GetResourceManager()->RegisterResourceName(this, name);
}
