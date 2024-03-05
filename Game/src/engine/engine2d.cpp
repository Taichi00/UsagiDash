#include "engine2d.h"
#include "engine/window.h"
#include "engine/buffer_manager.h"
#include "engine/buffer.h"
#include "game/game.h"
#include "game/resource/bitmap.h"
#include "game/resource/inline_image.h"
#include "game/resource/transition_effect.h"

Engine2D::Engine2D()
{
	render_target_width_ = 0;
	render_target_height_ = 0;
}

Engine2D::~Engine2D()
{
}

bool Engine2D::Init()
{
	if (!CreateD3D11Device())
	{
		printf("D3D11のデバイス生成に失敗\n");
		return false;
	}
	
	if (!CreateIWICImagingFactory())
	{
		printf("WICImagingFactoryの生成に失敗\n");
		return false;
	}

	if (!CreateD2DDeviceContext())
	{
		printf("D2DDeviceContextの生成に失敗\n");
		return false;
	}

	if (!CreateDWriteFactory())
	{
		printf("DWriteFactoryの生成に失敗\n");
		return false;
	}

	if (!CreateFontSetBuilder())
	{
		printf("FontSetBuilderの生成に失敗\n");
		return false;
	}

	if (!CreateD2DRenderTarget())
	{
		printf("D2dRenderTargetの生成に失敗\n");
		return false;
	}

	// カスタムエフェクトを登録
	auto hr = TransitionEffect::Register(d2d_factory_.Get());
	
	printf("2D描画エンジンの初期化に成功\n");
	return true;
}

void Engine2D::BeginRenderD2D() const
{
	auto currentBackBufferIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();

	const auto& wrappedBackBuffer = wrapped_back_buffers_[currentBackBufferIndex];
	const auto& backBufferForD2D = d2d_render_targets_[currentBackBufferIndex];

	d3d11on12_device_->AcquireWrappedResources(wrappedBackBuffer.GetAddressOf(), 1);
	d2d_device_context_->SetTarget(backBufferForD2D.Get());
	d2d_device_context_->BeginDraw();
	d2d_device_context_->SetTransform(D2D1::Matrix3x2F::Identity());
}

void Engine2D::EndRenderD2D() const
{
	auto currentBackBufferIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();

	const auto& wrappedBackBuffer = wrapped_back_buffers_[currentBackBufferIndex];

	d2d_device_context_->EndDraw();
	d3d11on12_device_->ReleaseWrappedResources(wrappedBackBuffer.GetAddressOf(), 1);
	d3d11_device_context_->Flush();
}

bool Engine2D::LoadCustomFonts(const std::vector<std::wstring>& fonts)
{
	for (const auto& path : fonts)
	{
		// フォントファイルの追加
		IDWriteFontFile* font_file;
		auto hr = dwrite_factory_->CreateFontFileReference(path.c_str(), nullptr, &font_file);
		if (SUCCEEDED(hr))
		{
			font_set_builder_->AddFontFile(font_file);
		}
	}
	
	// フォントセットの生成
	IDWriteFontSet* font_set;
	auto hr = font_set_builder_->CreateFontSet(&font_set);
	if (FAILED(hr))
	{
		return false;
	}
	
	// フォントコレクションの生成
	hr = dwrite_factory_->CreateFontCollectionFromFontSet(font_set, &font_collection_);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

void Engine2D::LoadBitmapFromFile(const std::wstring& path, ID2D1Bitmap** bitmap)
{
	ComPtr<IWICBitmapDecoder> decoder;
	ComPtr<IWICBitmapFrameDecode> source;
	ComPtr<IWICStream> stream;
	ComPtr<IWICFormatConverter> converter;
	ComPtr<IWICBitmapScaler> scaler;

	// decoder の作成
	auto hr = wic_factory_->CreateDecoderFromFilename(
		(LPCWSTR)path.c_str(),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&decoder
	);
	if (FAILED(hr))
		throw "CreateDecodeFromFilename";
	
	// 最初のフレームを source に保存
	hr = decoder->GetFrame(0, &source);
	if (FAILED(hr))
		throw "GetFrame";

	// 画像を 32bppPBGRA に変換するコンバータを生成
	hr = wic_factory_->CreateFormatConverter(&converter);
	if (FAILED(hr))
		throw "CreateFormatConverter";
	
	// 変換を実行
	hr = converter->Initialize(
		source.Get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		nullptr,
		0.f,
		WICBitmapPaletteTypeMedianCut
	);
	if (FAILED(hr))
		throw "Initialize";
	
	// bitmap を作成
	hr = d2d_device_context_->CreateBitmapFromWicBitmap(
		converter.Get(),
		nullptr,
		bitmap
	);
	if (FAILED(hr))
		throw "CreateBitmapFromWicBitmap";
}

Vec2 Engine2D::GetTextSize(const Text& text)
{
	const auto& text_format = text_format_map_.at(text.prop.font);

	IDWriteTextLayout* layout = nullptr;
	dwrite_factory_->CreateTextLayout(text.string.c_str(), (UINT32)text.string.size(), text_format.Get(), 10000, 0, &layout);

	CreateTextLayout(text, {}, layout);

	DWRITE_TEXT_METRICS text_metrics;
	layout->GetMetrics(&text_metrics);

	layout->Release();

	return Vec2(text_metrics.width, text_metrics.height);
}

void Engine2D::SetTransform(const Vec2& position, const float rotation, const Vec2& scale)
{
	auto mat = D2D1::Matrix3x2F::Scale(scale.x, scale.y)
		* D2D1::Matrix3x2F::Rotation(rotation)
		* D2D1::Matrix3x2F::Translation(position.x, position.y);

	d2d_device_context_->SetTransform(mat);
}

void Engine2D::SetTransform(const Matrix3x2& matrix)
{
	D2D1::Matrix3x2F mat;
	mat.m11 = matrix.m11; mat.m12 = matrix.m12;
	mat.m21 = matrix.m21; mat.m22 = matrix.m22;
	mat.dx = matrix.m31; mat.dy = matrix.m32;

	d2d_device_context_->SetTransform(mat);
}

void Engine2D::DrawText(const Text& text, const Rect2& rect, const Color& color) const
{
	const auto& text_format = text_format_map_.at(text.prop.font);
	//const auto& brush = solid_color_brush_map_.at(color);
	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	d2d_device_context_->CreateSolidColorBrush(
		{ color.r, color.g, color.b, color.a },
		brush.GetAddressOf()
	);

	// text layout の生成
	IDWriteTextLayout* layout = nullptr;
	auto hr = dwrite_factory_->CreateTextLayout(
		text.string.c_str(),
		(UINT32)text.string.size(),
		text_format.Get(),
		rect.Width(),
		rect.Height(),
		&layout
	);
	if (FAILED(hr))
		throw "CreateTextLayout";

	CreateTextLayout(text, color, layout);

	// 描画
	d2d_device_context_->DrawTextLayout(
		{ rect.left, rect.top },
		layout,
		brush.Get(),
		D2D1_DRAW_TEXT_OPTIONS_DISABLE_COLOR_BITMAP_SNAPPING
	);

	layout->Release();
}

void Engine2D::DrawRectangle(const Rect2& rect, const Color& color, const float radius) const
{
	//const auto& brush = solid_color_brush_map_.at(color);
	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	d2d_device_context_->CreateSolidColorBrush(
		{ color.r, color.g, color.b, color.a },
		brush.GetAddressOf()
	);

	D2D1_RECT_F rectf = { rect.left, rect.top, rect.right, rect.bottom };

	if (!radius)
	{
		d2d_device_context_->DrawRectangle(rectf, brush.Get());
	}
	else
	{
		d2d_device_context_->DrawRoundedRectangle({ rectf, radius, radius }, brush.Get());
	}
}

void Engine2D::DrawFillRectangle(const Rect2& rect, const Color& color, const float radius) const
{
	//const auto& brush = solid_color_brush_map_.at(color);
	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	d2d_device_context_->CreateSolidColorBrush(
		{ color.r, color.g, color.b, color.a },
		brush.GetAddressOf()
	);

	D2D1_RECT_F rectf = { rect.left, rect.top, rect.right, rect.bottom };

	if (!radius)
	{
		d2d_device_context_->FillRectangle(rectf, brush.Get());
	}
	else
	{
		d2d_device_context_->FillRoundedRectangle({ rectf, radius, radius }, brush.Get());
	}
}

void Engine2D::DrawTransition(const Color& color, const float time, const Vec2 direction, const bool inverse) const
{
	// 現在の transform 行列を保存しておく
	D2D1_MATRIX_3X2_F transform;
	d2d_device_context_->GetTransform(&transform);

	// 一時レンダーターゲットを生成
	ComPtr<ID2D1BitmapRenderTarget> bitmap_render_target;
	d2d_device_context_->CreateCompatibleRenderTarget(
		{ (float)render_target_width_, (float)render_target_height_ },
		&bitmap_render_target
	);

	bitmap_render_target->BeginDraw();
	bitmap_render_target->Clear({ 0, 0, 0, 0 });
	bitmap_render_target->EndDraw();

	ComPtr<ID2D1Bitmap> render_bitmap;
	bitmap_render_target->GetBitmap(&render_bitmap);

	// エフェクトを生成
	ComPtr<ID2D1Effect> effect;
	auto hr = d2d_device_context_->CreateEffect(CLSID_TransitionEffect, &effect);
	
	D2D1_VECTOR_4F color_vec = { color.r, color.g, color.b, color.a };
	D2D1_VECTOR_2F dir = { direction.x, direction.y };
	effect->SetValue(TRANSITION_PROP_COLOR, color_vec);
	effect->SetValue(TRANSITION_PROP_TIME, time);
	effect->SetValue(TRANSITION_PROP_ASPECT, (float)render_target_height_ / (float)render_target_width_);
	effect->SetValue(TRANSITION_PROP_DIRECTION, dir);
	effect->SetValue(TRANSITION_PROP_INVERSE, (float)inverse);
	effect->SetInput(0, render_bitmap.Get());
	
	d2d_device_context_->SetTransform(D2D1::Matrix3x2F::Identity());
	d2d_device_context_->DrawImage(effect.Get());

	// transform 行列を元に戻す
	d2d_device_context_->SetTransform(transform);
}

void Engine2D::DrawBitmap(const Bitmap* bitmap)
{
	d2d_device_context_->DrawBitmap(
		bitmap->Data().Get(),
		0,
		1.f,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
		);
}

void Engine2D::ResetRenderTargets()
{
	for (auto i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		wrapped_back_buffers_[i].Reset();
		d2d_render_targets_[i].Reset();
	}
	d2d_device_context_->SetTarget(nullptr);
	//d2d_device_context_->Flush();
	d3d11_device_context_->Flush();
}

Vec2 Engine2D::RenderTargetSize()
{
	return Vec2((float)render_target_width_, (float)render_target_height_);
}

bool Engine2D::CreateD3D11Device()
{
	ComPtr<ID3D11Device> d3d11Device;
	UINT d3d11DeviceFlags = 0U;

#ifdef _DEBUG
	d3d11DeviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#else
	d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

	auto queue = Game::Get()->GetEngine()->QueueAddress();

	auto hr = D3D11On12CreateDevice(
		Game::Get()->GetEngine()->Device(),
		d3d11DeviceFlags,
		nullptr,
		0U,
		reinterpret_cast<IUnknown**>(queue),
		1U,
		0U,
		&d3d11Device,
		&d3d11_device_context_,
		nullptr
	);
	if (FAILED(hr))
	{
		return false;
	}

	d3d11Device.As(&d3d11on12_device_);
	d3d11_device_ = d3d11Device;

	return true;
}

bool Engine2D::CreateD2DDeviceContext()
{
	constexpr D2D1_FACTORY_OPTIONS factoryOptions{};

	auto hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory3),
		&factoryOptions,
		&d2d_factory_
	);
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	hr = d3d11on12_device_.As(&dxgiDevice);
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<ID2D1Device> d2dDevice = nullptr;
	hr = d2d_factory_->CreateDevice(
		dxgiDevice.Get(),
		d2dDevice.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
	{
		return false;
	}
	
	hr = d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		d2d_device_context_.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Engine2D::CreateDWriteFactory()
{
	// テキストフォーマットを生成するためのファクトリであるIDWriteFactoryを生成
	auto hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory5), &dwrite_factory_
	);

	return SUCCEEDED(hr);
}

bool Engine2D::CreateFontSetBuilder()
{
	auto hr = dwrite_factory_->CreateFontSetBuilder(&font_set_builder_);

	return SUCCEEDED(hr);
}

bool Engine2D::CreateIWICImagingFactory()
{
	// WICImagingFactoryの生成
	auto hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&wic_factory_)
	);

	return SUCCEEDED(hr);
}

void Engine2D::CreateTextLayout(const Text& text, const Color& color, IDWriteTextLayout* layout) const
{
	auto rm = Game::Get()->GetResourceManager();

	DWRITE_TEXT_RANGE range = { 0, (unsigned)text.string.size() };

	layout->SetFontSize(text.prop.font_size, range);
	layout->SetFontWeight((DWRITE_FONT_WEIGHT)text.prop.font_weight, range);
	layout->SetTextAlignment((DWRITE_TEXT_ALIGNMENT)text.prop.horizontal_alignment);
	layout->SetParagraphAlignment((DWRITE_PARAGRAPH_ALIGNMENT)text.prop.vertical_alignment);

	for (const auto& tag : text.FormatTags())
	{
		// アイコン
		if (tag.key == L"bitmap")
		{
			auto inline_image = new InlineImage(
				d2d_device_context_.Get(), 
				(Bitmap*)rm->GetResourceFromName(tag.value), 
				text.prop.font_size * 1.3f, 
				color);
			layout->SetInlineObject(inline_image, { tag.range.start, tag.range.length });
		}
	}
}

void Engine2D::RegisterSolidColorBrush(const Color& color)
{
	if (solid_color_brush_map_.contains(color))
	{
		return;
	}

	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	d2d_device_context_->CreateSolidColorBrush(
		{ color.r, color.g, color.b, color.a },
		brush.GetAddressOf()
	);

	solid_color_brush_map_[color] = brush;
}

void Engine2D::RegisterTextFormat(const std::wstring& font_name)
{
	if (text_format_map_.contains(font_name))
	{
		return;
	}
	
	ComPtr<IDWriteTextFormat> text_format = nullptr;
	dwrite_factory_->CreateTextFormat(
		font_name.c_str(),
		font_collection_.Get(),
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16,
		L"ja-jp",
		text_format.GetAddressOf()
	);

	text_format_map_[font_name] = text_format;
}

bool Engine2D::CreateD2DRenderTarget()
{
	auto engine = Game::Get()->GetEngine();

	D3D11_RESOURCE_FLAGS flags = { D3D11_BIND_RENDER_TARGET };

	float dpi = (float)GetDpiForWindow(Game::Get()->GetEngine()->GetWindow()->HWnd());

	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		static_cast<float>(dpi),
		static_cast<float>(dpi)
	);

	// GBufferを生成
	/*GBufferProperty propRGBA{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		engine->FrameBufferWidth(), engine->FrameBufferHeight(),
		engine->GBufferHeap().get(), engine->RtvHeap().get()
	};

	auto gm = Game::Get()->GetEngine()->GetGBufferManager();
	gm->CreateGBuffer("2DLayer", propRGBA);*/

	// WrappedResourceを生成
	for (UINT i = 0U; i < Engine::FRAME_BUFFER_COUNT; ++i)
	{
		ComPtr<ID3D11Resource> wrappedBackBuffer = nullptr;

		auto hr = d3d11on12_device_->CreateWrappedResource(
			engine->RenderTarget(i),
			&flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(wrappedBackBuffer.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr))
		{
			return false;
		}

		ComPtr<IDXGISurface> dxgiSurface = nullptr;
		hr = wrappedBackBuffer.As(&dxgiSurface);
		if (FAILED(hr))
		{
			return false;
		}

		ComPtr<ID2D1Bitmap1> d2dRenderTarget = nullptr;
		hr = d2d_device_context_->CreateBitmapFromDxgiSurface(
			dxgiSurface.Get(),
			&bitmapProperties,
			&d2dRenderTarget
		);
		if (FAILED(hr))
		{
			return false;
		}

		wrapped_back_buffers_[i] = wrappedBackBuffer;
		d2d_render_targets_[i] = d2dRenderTarget;
	}

	auto size = d2d_render_targets_[0]->GetSize();
	render_target_width_ = (unsigned int)size.width;
	render_target_height_ = (unsigned int)size.height;

	render_target_scale_ = render_target_height_ / DEFAULT_HEIGHT;

	return true;
}

