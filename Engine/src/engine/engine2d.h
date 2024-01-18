#pragma once

#include "engine/comptr.h"
#include "engine/engine.h"
#include <d2d1_3.h>
#include <d3d11_4.h>
#include <d3d11on12.h>
#include <dwrite.h>
#include <unordered_map>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#undef DrawText

class Engine2D
{
public:
	Engine2D();
	~Engine2D();

	bool Init();	// ������

	bool CreateD2DRenderTarget();	// Direct2D�p�̃����_�[�^�[�Q�b�g�𐶐�

	void BeginRenderD2D() const;	// D2D�̕`��J�n
	void EndRenderD2D() const;	// D2D�̕`��I��

	void RegisterSolidColorBrush(const std::string& key, const D2D1::ColorF color) noexcept;	// Brush��o�^
	void RegisterTextFormat(const std::string& key, const std::wstring& fontName, const FLOAT fontSize) noexcept;	// �e�L�X�g�t�H�[�}�b�g��o�^
	void DrawText(const std::string& textFormatKey, const std::string& solidColorBrushKey,
		const std::wstring& text, const D2D1_RECT_F& rect) const noexcept;

	void ResetRenderTargets();

private:
	bool CreateD3D11Device();		// D3D11Device�𐶐�
	bool CreateD2DDeviceContext();	// D2D�f�o�C�X�R���e�L�X�g�𐶐�
	bool CreateDWriteFactory();		// DirectWrite�̃t�@�N�g���𐶐�

private:
	ComPtr<ID3D11DeviceContext> d3d11_device_context_;	// D3D11�̃f�o�C�X�R���e�L�X�g
	ComPtr<ID3D11On12Device> d3d11on12_device_;			// D3D11On12�̃f�o�C�X�iDirect2D���g�p���邽�߂ɕK�v�j
	ComPtr<ID3D11Device> d3d11_device_;					// D3D11�̃f�o�C�X
	ComPtr<IDWriteFactory> dwrite_factory_;				// DirectWrite�̃t�@�N�g��
	ComPtr<ID2D1DeviceContext> d2d_device_context_;		// D2D1�̃f�o�C�X�R���e�L�X�g

private:
	ComPtr<ID3D11Resource> wrapped_back_buffers_[Engine::FRAME_BUFFER_COUNT];
	ComPtr<ID2D1Bitmap1> d2d_render_targets_[Engine::FRAME_BUFFER_COUNT];

	std::unordered_map<std::string, ComPtr<ID2D1SolidColorBrush>> solid_color_brush_map_;
	std::unordered_map<std::string, ComPtr<IDWriteTextFormat>> text_format_map_;
};