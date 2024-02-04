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

	if (!CreateD2DRenderTarget())
	{
		printf("D2dRenderTargetの生成に失敗\n");
		return false;
	}

	//RegisterTextFormat("normal", L"MS ゴシック", 24);
	//RegisterSolidColorBrush("white", D2D1::ColorF::White);

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

void Engine2D::RegisterSolidColorBrush(const std::string& key, const D2D1::ColorF color) noexcept
{
	if (solid_color_brush_map_.contains(key))
	{
		return;
	}

	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	d2d_device_context_->CreateSolidColorBrush(
		D2D1::ColorF(color),
		brush.GetAddressOf()
	);

	solid_color_brush_map_[key] = brush;
}

void Engine2D::RegisterTextFormat(const std::string& key, const std::wstring& fontName, const FLOAT fontSize) noexcept
{
	if (text_format_map_.contains(key))
	{
		return;
	}

	ComPtr<IDWriteTextFormat> textFormat = nullptr;
	dwrite_factory_->CreateTextFormat(
		fontName.c_str(),
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"ja-jp",
		textFormat.GetAddressOf()
	);

	text_format_map_[key] = textFormat;
}

void Engine2D::DrawText(const std::string& textFormatKey, const std::string& solidColorBrushKey, const std::wstring& text, const D2D1_RECT_F& rect) const noexcept
{
	const auto& textFormat = text_format_map_.at(textFormatKey);
	const auto& solidColorBrush = solid_color_brush_map_.at(solidColorBrushKey);

	d2d_device_context_->DrawTextW(
		text.c_str(),
		static_cast<UINT32>(text.length()),
		textFormat.Get(),
		&rect,
		solidColorBrush.Get()
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
		__uuidof(IDWriteFactory), &dwrite_factory_
	);

	return SUCCEEDED(hr);
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

	return true;
}

