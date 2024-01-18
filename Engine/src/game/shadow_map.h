#pragma once
#include <d3d12.h>
#include "engine/comptr.h"
#include "engine/descriptor_heap.h"

class RootSignature;
class PipelineState;

class ShadowMap
{
public:
	ShadowMap();
	~ShadowMap();

	void BeginRender();
	void EndRender();

	ID3D12Resource* Resource();
	const DescriptorHandle& SrvHandle();

private:
	bool CreateShadowBuffer();
	bool PreparePSO();
	bool PrepareRootSignature();

	void BlurHorizontal();
	void BlurVertical();

private:
	/*ComPtr<ID3D12Resource> color_[2];
	ComPtr<ID3D12Resource> depth_;

	DescriptorHandle rtv_handle_[2];
	DescriptorHandle dsv_handle_;
	DescriptorHandle srv_handle_[2];*/

	RootSignature* root_signature_;

	PipelineState* blur_horizontal_pso_;
	PipelineState* blur_vertical_pso_;

	int width_ = 1024;
	int height_ = 1024;

};