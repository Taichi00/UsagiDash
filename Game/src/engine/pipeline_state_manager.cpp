#include "pipeline_state_manager.h"
#include "engine/pipeline_state.h"

PipelineState* PipelineStateManager::Create(const std::string& name)
{
	map_[name] = std::make_unique<PipelineState>();
	return map_[name].get();
}

PipelineState* PipelineStateManager::Get(const std::string& name)
{
	return map_[name].get();
}
