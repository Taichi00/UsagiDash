#pragma once

#include "game/component/mesh_renderer.h"
#include <memory>

class Texture2D;

class BillboardRenderer : public MeshRenderer
{
public:
	BillboardRenderer(std::shared_ptr<Texture2D> albedoTexture, std::shared_ptr<Texture2D> normalTexture);
	~BillboardRenderer();

	void Update() override;
	
private:
	void UpdateCB();

private:
	std::shared_ptr<Texture2D> albedo_texture_;
	std::shared_ptr<Texture2D> normal_texture_;

	float width_, height_;
};