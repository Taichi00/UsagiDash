#include "engine2d.h"
#include "engine/window.h"
#include "engine/buffer_manager.h"
#include "engine/buffer.h"
#include "game/game.h"

Engine2D::Engine2D()
{
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
}

Vec2 Engine2D::GetTextSize(const std::wstring& text, const std::wstring& font, const float size, const unsigned int weight)
{
	const auto& text_format = CreateTextFormat(font, size, weight);

	IDWriteTextLayout* text_layout = NULL;
	dwrite_factory_->CreateTextLayout(text.c_str(), text.size(), text_format.Get(), 10000, 0, &text_layout);

	DWRITE_TEXT_METRICS text_metrics;
	text_layout->GetMetrics(&text_metrics);

	text_layout->Release();

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

void Engine2D::DrawText(const std::wstring& text, const Rect2& rect, const std::wstring& font, const float size, const unsigned int weight, const Color& color) const
{
	const auto& text_format = CreateTextFormat(font, size, weight);
	const auto& brush = CreateSolidColorBrush(color);
	
	/*IDWriteTextLayout* layout;
	dwrite_factory_->CreateTextLayout(
		text.c_str(),
		text.size(),
		text_format.Get(),
		rect.Width(),
		rect.Height(),
		&layout
	);

	ComPtr<ID2D1Effect> effect;
	d2d_device_context_->CreateEffect(CLSID_D2D1GaussianBlur, &effect);
	effect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 3.0f);

	layout->SetDrawingEffect(effect.Get(), { 0, (unsigned int)text.size() });*/

	D2D1_RECT_F rectf = { rect.left, rect.top, rect.right, rect.bottom };
	
	d2d_device_context_->DrawTextW(
		text.c_str(),
		static_cast<UINT32>(text.length()),
		text_format.Get(),
		&rectf,
		brush.Get()
	);
	/*d2d_device_context_->DrawTextLayout(
		{ rect.left, rect.top },
		layout,
		brush.Get()
	);

	layout->Release();*/
}

void Engine2D::DrawRectangle(const Rect2& rect, const Color& color) const
{
	const auto& brush = CreateSolidColorBrush(color);

	d2d_device_context_->DrawRectangle(
		{ rect.left, rect.top, rect.right, rect.bottom },
		brush.Get()
	);
}

void Engine2D::DrawFillRectangle(const Rect2& rect, const Color& color) const
{
	const auto& brush = CreateSolidColorBrush(color);

	d2d_device_context_->FillRectangle(
		{ rect.left, rect.top, rect.right, rect.bottom },
		brush.Get()
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
	return Vec2(render_target_width_, render_target_height_);
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
	ComPtr<ID2D1Factory3> d2dfactory = nullptr;
	constexpr D2D1_FACTORY_OPTIONS factoryOptions{};

	auto hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory3),
		&factoryOptions,
		&d2dfactory
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
	hr = d2dfactory->CreateDevice(
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

ComPtr<ID2D1SolidColorBrush> Engine2D::CreateSolidColorBrush(const Color& color) const
{
	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	d2d_device_context_->CreateSolidColorBrush(
		{ color.r, color.g, color.b, color.a },
		brush.GetAddressOf()
	);

	return brush;
}

ComPtr<IDWriteTextFormat> Engine2D::CreateTextFormat(const std::wstring& font_name, const float font_size, const unsigned int font_weight) const
{
	ComPtr<IDWriteTextFormat> text_format = nullptr;
	dwrite_factory_->CreateTextFormat(
		font_name.c_str(),
		font_collection_.Get(),
		(DWRITE_FONT_WEIGHT)font_weight,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		font_size,
		L"ja-jp",
		text_format.GetAddressOf()
	);

	return text_format;
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
	render_target_width_ = size.width;
	render_target_height_ = size.height;

	return true;
}

