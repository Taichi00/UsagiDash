#pragma once

#include <string>
#include <map>
#include <memory>

class PipelineState;

class PipelineStateManager
{
public:
	PipelineStateManager() = default;
	~PipelineStateManager() = default;

	PipelineState* Create(const std::string& name);

	PipelineState* Get(const std::string& name);

private:
	std::map<std::string, std::unique_ptr<PipelineState>> map_;
};