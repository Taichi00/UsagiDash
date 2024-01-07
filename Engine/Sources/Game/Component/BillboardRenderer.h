#pragma once

#include "MeshRenderer.h"
#include <memory>

class Texture2D;

class BillboardRenderer : public MeshRenderer
{
public:
	BillboardRenderer(std::shared_ptr<Texture2D> albedoTexture, std::shared_ptr<Texture2D> normalTexture);
	~BillboardRenderer();

	void Update() override;
	void DrawShadow() override;
	void DrawDepth() override;

private:
	void UpdateCB();

private:
	std::shared_ptr<Texture2D> m_pTexture;
	std::shared_ptr<Texture2D> m_pNormalTexture;

	float m_width, m_height;
};