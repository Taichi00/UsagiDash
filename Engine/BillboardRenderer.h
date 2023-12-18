#pragma once

#include "MeshRenderer.h"

class Texture2D;

class BillboardRenderer : public MeshRenderer
{
public:
	BillboardRenderer(Texture2D* albedoTexture, Texture2D* normalTexture);
	~BillboardRenderer();

	void Update() override;
	void DrawShadow() override;
	void DrawDepth() override;

private:
	void UpdateCB();

private:
	Texture2D* m_pTexture;
	Texture2D* m_pNormalTexture;

	float m_width, m_height;
};