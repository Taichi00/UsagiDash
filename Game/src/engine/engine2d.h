#pragma once

#include "engine/comptr.h"
#include "engine/engine.h"
#include "math/vec.h"
#include "math/rect.h"
#include "math/color.h"
#include "math/matrix3x2.h"
#include "game/component/gui/text.h"
#include <d2d1_3.h>
#include <d2d1_3helper.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <d3d11_4.h>
#include <d3d11on12.h>
#include <dwrite_3.h>
#include <unordered_map>
#include <vector>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#undef DrawText

class Bitmap;
struct InlineImage;

class Engine2D
{
public:
	Engine2D();
	~Engine2D();

	bool Init();	// ������

	bool CreateD2DRenderTarget();	// Direct2D�p�̃����_�[�^�[�Q�b�g�𐶐�

	void BeginRenderD2D() const;	// D2D�̕`��J�n
	void EndRenderD2D() const;	// D2D�̕`��I��

	// �J�X�^���t�H���g��ǂݍ���
	bool LoadCustomFonts(const std::vector<std::wstring>& fonts);

	// �摜����r�b�g�}�b�v�f�[�^���擾����
	void LoadBitmapFromFile(const std::wstring& path, ID2D1Bitmap** bitmap);

	// �e�L�X�g���҂�������܂��`�̃T�C�Y���擾����
	Vec2 GetTextSize(const Text& text);

	// �ϊ��s���ݒ肷��
	void SetTransform(const Vec2& position, const float rotation, const Vec2& scale);
	void SetTransform(const Matrix3x2& matrix);

	// ������`�悷��
	void DrawText(const Text& text, const Rect2& rect, const Color& color) const;

	// ��`��`�悷��
	void DrawRectangle(const Rect2& rect, const Color& color, const float radius) const;
	void DrawFillRectangle(const Rect2& rect, const Color& color, const float radius) const;

	// �g�����W�V�����p�̋�`��`�悷��
	void DrawTransition(const Color& color, const float time, const Vec2 direction, const bool inverse) const;

	// �摜��`�悷��
	void DrawBitmap(const Bitmap* bitmap, const Color& color);

	// �����_�[�^�[�Q�b�g������������
	void ResetRenderTargets();

	Vec2 RenderTargetSize();

	void RegisterSolidColorBrush(const Color& color);
	void RegisterTextFormat(const std::wstring& font_name);

	float RenderTargetScale() const { return render_target_scale_; }

private:
	bool CreateD3D11Device();		// D3D11Device�𐶐�
	bool CreateD2DDeviceContext();	// D2D�f�o�C�X�R���e�L�X�g�𐶐�
	bool CreateDWriteFactory();		// DirectWrite�̃t�@�N�g���𐶐�
	bool CreateFontSetBuilder();	// FontSetBuilder�𐶐��i�t�H���g�t�@�C���̓ǂݍ��݂ɕK�v�j
	bool CreateIWICImagingFactory(); // IWICImagingFactory�𐶐��i�摜�t�@�C���̓ǂݍ��݂ɕK�v�j

	void CreateTextLayout(const Text& text, const Color& color, IDWriteTextLayout* layout) const;

private:
	ComPtr<ID3D11DeviceContext> d3d11_device_context_;	// D3D11�̃f�o�C�X�R���e�L�X�g
	ComPtr<ID3D11On12Device> d3d11on12_device_;			// D3D11On12�̃f�o�C�X�iDirect2D���g�p���邽�߂ɕK�v�j
	ComPtr<ID3D11Device> d3d11_device_;					// D3D11�̃f�o�C�X
	ComPtr<IDWriteFactory5> dwrite_factory_;			// DirectWrite�̃t�@�N�g��
	ComPtr<IDWriteFontSetBuilder1> font_set_builder_;	// �t�H���g�Z�b�g�̐����p
	ComPtr<ID2D1DeviceContext> d2d_device_context_;		// D2D�̃f�o�C�X�R���e�L�X�g
	ComPtr<ID2D1Factory3> d2d_factory_;					// D2D�̃t�@�N�g��

	ComPtr<IDWriteFontCollection1> font_collection_;	// �t�H���g�R���N�V����

	ComPtr<IWICImagingFactory2> wic_factory_;

private:
	ComPtr<ID3D11Resource> wrapped_back_buffers_[Engine::FRAME_BUFFER_COUNT];
	ComPtr<ID2D1Bitmap1> d2d_render_targets_[Engine::FRAME_BUFFER_COUNT];

	const float DEFAULT_HEIGHT = 720.0f;
	float render_target_scale_ = 1.f; // default height �ɑ΂��錻�݂� render target height �̔{��

	unsigned int render_target_width_;
	unsigned int render_target_height_;

	std::unordered_map<Color, ComPtr<ID2D1SolidColorBrush>, Color::HashFunctor> solid_color_brush_map_;
	std::unordered_map<std::wstring, ComPtr<IDWriteTextFormat>> text_format_map_;
};