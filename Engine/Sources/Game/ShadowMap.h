#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include "ComPtr.h"
#include "DescriptorHeap.h"

class RootSignature;
class PipelineState;

class ShadowMap
{
public:
	ShadowMap();

	void BeginRender();
	void EndRender();

	ID3D12Resource* Resource();
	DescriptorHandle* SrvHandle();

private:
	bool CreateShadowBuffer();
	bool PreparePSO();
	bool PrepareRootSignature();

	void BlurHorizontal();
	void BlurVertical();

private:
	ComPtr<ID3D12Resource> m_pColor[2];
	ComPtr<ID3D12Resource> m_pDepth;

	DescriptorHandle m_pRtvHandle[2];
	DescriptorHandle m_pDsvHandle;
	DescriptorHandle m_pSrvHandle[2];

	RootSignature* m_pRootSignature;

	PipelineState* m_pBlurHorizontalPSO;
	PipelineState* m_pBlurVerticalPSO;

	int m_width = 1024;
	int m_height = 1024;

};