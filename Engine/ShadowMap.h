#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include "ComPtr.h"

class DescriptorHeap;
class DescriptorHandle;

class ShadowMap
{
public:
	ShadowMap();

	void BeginRender();
	void EndRender();

	ID3D12Resource* Resource();

private:
	bool CreateShadowBuffer();

	ComPtr<ID3D12Resource> m_pColor;
	ComPtr<ID3D12Resource> m_pDepth;

	DescriptorHandle* m_pRtvHandle;
	DescriptorHandle* m_pDsvHandle;

};