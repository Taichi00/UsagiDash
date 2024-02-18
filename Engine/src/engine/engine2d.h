#pragma once

#include "engine/comptr.h"
#include "engine/engine.h"
#include "math/vec.h"
#include "math/rect.h"
#include "math/color.h"
#include "math/matrix3x2.h"
#include <d2d1_3.h>
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

class Font;

class Engine2D
{
public:
	Engine2D();
	~Engine2D();

	bool Init();	// 初期化

	bool CreateD2DRenderTarget();	// Direct2D用のレンダーターゲットを生成

	void BeginRenderD2D() const;	// D2Dの描画開始
	void EndRenderD2D() const;	// D2Dの描画終了

	bool LoadCustomFonts(const std::vector<std::wstring>& fonts);

	Vec2 GetTextSize(const std::wstring& text, const std::wstring& font, const float size, const unsigned int weight);

	void SetTransform(const Vec2& position, const float rotation, const Vec2& scale);
	void SetTransform(const Matrix3x2& matrix);

	void DrawText(const std::wstring& text, const Rect2& rect,
		const std::wstring& font, const float size, const unsigned int weight, const Color& color) const;
	
	void DrawRectangle(const Rect2& rect, const Color& color) const;
	void DrawFillRectangle(const Rect2& rect, const Color& color) const;

	void ResetRenderTargets();

	Vec2 RenderTargetSize();

private:
	bool CreateD3D11Device();		// D3D11Deviceを生成
	bool CreateD2DDeviceContext();	// D2Dデバイスコンテキストを生成
	bool CreateDWriteFactory();		// DirectWriteのファクトリを生成
	bool CreateFontSetBuilder();	// FontSetBuilderを生成（フォントファイルの読み込みに必要）

	ComPtr<ID2D1SolidColorBrush> CreateSolidColorBrush(const Color& color) const;
	ComPtr<IDWriteTextFormat> CreateTextFormat(const std::wstring& font_name, const float font_size, const unsigned int font_weight) const;

private:
	ComPtr<ID3D11DeviceContext> d3d11_device_context_;	// D3D11のデバイスコンテキスト
	ComPtr<ID3D11On12Device> d3d11on12_device_;			// D3D11On12のデバイス（Direct2Dを使用するために必要）
	ComPtr<ID3D11Device> d3d11_device_;					// D3D11のデバイス
	ComPtr<IDWriteFactory5> dwrite_factory_;			// DirectWriteのファクトリ
	ComPtr<IDWriteFontSetBuilder1> font_set_builder_;	// フォントセットの生成用
	ComPtr<ID2D1DeviceContext> d2d_device_context_;		// D2D1のデバイスコンテキスト

	ComPtr<IDWriteFontCollection1> font_collection_;	// フォントコレクション

private:
	ComPtr<ID3D11Resource> wrapped_back_buffers_[Engine::FRAME_BUFFER_COUNT];
	ComPtr<ID2D1Bitmap1> d2d_render_targets_[Engine::FRAME_BUFFER_COUNT];

	unsigned int render_target_width_;
	unsigned int render_target_height_;

	//std::unordered_map<std::string, ComPtr<ID2D1SolidColorBrush>> solid_color_brush_map_;
	//std::unordered_map<std::string, ComPtr<IDWriteTextFormat>> text_format_map_;
};