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

	bool Init();	// 初期化

	bool CreateD2DRenderTarget();	// Direct2D用のレンダーターゲットを生成

	void BeginRenderD2D() const;	// D2Dの描画開始
	void EndRenderD2D() const;	// D2Dの描画終了

	// カスタムフォントを読み込む
	bool LoadCustomFonts(const std::vector<std::wstring>& fonts);

	// 画像からビットマップデータを取得する
	void LoadBitmapFromFile(const std::wstring& path, ID2D1Bitmap** bitmap);

	// テキストがぴったり収まる矩形のサイズを取得する
	Vec2 GetTextSize(const Text& text);

	// 変換行列を設定する
	void SetTransform(const Vec2& position, const float rotation, const Vec2& scale);
	void SetTransform(const Matrix3x2& matrix);

	// 文字を描画する
	void DrawText(const Text& text, const Rect2& rect, const Color& color) const;

	// 矩形を描画する
	void DrawRectangle(const Rect2& rect, const Color& color, const float radius) const;
	void DrawFillRectangle(const Rect2& rect, const Color& color, const float radius) const;

	// トランジション用の矩形を描画する
	void DrawTransition(const Color& color, const float time, const Vec2 direction, const bool inverse) const;

	// 画像を描画する
	void DrawBitmap(const Bitmap* bitmap, const Color& color);

	// レンダーターゲットを初期化する
	void ResetRenderTargets();

	Vec2 RenderTargetSize();

	void RegisterSolidColorBrush(const Color& color);
	void RegisterTextFormat(const std::wstring& font_name);

	float RenderTargetScale() const { return render_target_scale_; }

private:
	bool CreateD3D11Device();		// D3D11Deviceを生成
	bool CreateD2DDeviceContext();	// D2Dデバイスコンテキストを生成
	bool CreateDWriteFactory();		// DirectWriteのファクトリを生成
	bool CreateFontSetBuilder();	// FontSetBuilderを生成（フォントファイルの読み込みに必要）
	bool CreateIWICImagingFactory(); // IWICImagingFactoryを生成（画像ファイルの読み込みに必要）

	void CreateTextLayout(const Text& text, const Color& color, IDWriteTextLayout* layout) const;

private:
	ComPtr<ID3D11DeviceContext> d3d11_device_context_;	// D3D11のデバイスコンテキスト
	ComPtr<ID3D11On12Device> d3d11on12_device_;			// D3D11On12のデバイス（Direct2Dを使用するために必要）
	ComPtr<ID3D11Device> d3d11_device_;					// D3D11のデバイス
	ComPtr<IDWriteFactory5> dwrite_factory_;			// DirectWriteのファクトリ
	ComPtr<IDWriteFontSetBuilder1> font_set_builder_;	// フォントセットの生成用
	ComPtr<ID2D1DeviceContext> d2d_device_context_;		// D2Dのデバイスコンテキスト
	ComPtr<ID2D1Factory3> d2d_factory_;					// D2Dのファクトリ

	ComPtr<IDWriteFontCollection1> font_collection_;	// フォントコレクション

	ComPtr<IWICImagingFactory2> wic_factory_;

private:
	ComPtr<ID3D11Resource> wrapped_back_buffers_[Engine::FRAME_BUFFER_COUNT];
	ComPtr<ID2D1Bitmap1> d2d_render_targets_[Engine::FRAME_BUFFER_COUNT];

	const float DEFAULT_HEIGHT = 720.0f;
	float render_target_scale_ = 1.f; // default height に対する現在の render target height の倍率

	unsigned int render_target_width_;
	unsigned int render_target_height_;

	std::unordered_map<Color, ComPtr<ID2D1SolidColorBrush>, Color::HashFunctor> solid_color_brush_map_;
	std::unordered_map<std::wstring, ComPtr<IDWriteTextFormat>> text_format_map_;
};