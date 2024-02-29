#include "inline_image.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "game/game.h"

InlineImage::InlineImage(ID2D1DeviceContext* device_context, Bitmap* bitmap, float size, const Color& color)
{
    device_context_ = device_context;
    device_context_->AddRef();

    bitmap_ = bitmap;
    
    auto bitmap_size = bitmap_->Size();
    float ratio = size / bitmap_size.y;

    size_ = size;
    baseline_ = 0.78f;
    color_ = color;

    engine_ = Game::Get()->GetEngine()->GetEngine2D();
}

HRESULT __stdcall InlineImage::Draw(
    __maybenull void* clientDrawingContext, 
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect)
{
    float ratio = engine_->AspectRatio();

    auto bitmap_size = bitmap_->Size();
    auto scale = size_ / bitmap_size.y;
    
    /*auto color = static_cast<ID2D1SolidColorBrush*>(clientDrawingEffect)->GetColor();
    printf("%f\n", color.r);*/
    //D2D1_RECT_F rect = {originX, originY, originX + size_.x, originY + size_.y};
    
    // êFïœä∑
    D2D1_MATRIX_5X4_F matrix = D2D1::Matrix5x4F(
        color_.r, 0, 0, 0,
        0, color_.g, 0, 0,
        0, 0, color_.b, 0,
        0, 0, 0, color_.a,
        0, 0, 0, 0
    );

    ComPtr<ID2D1Effect> effect;
    device_context_->CreateEffect(CLSID_D2D1ColorMatrix, &effect);

    effect->SetInput(0, bitmap_->Data().Get());
    effect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);

    // ç¿ïWïœä∑
    D2D1_MATRIX_3X2_F transform = {};
    D2D1_MATRIX_3X2_F curr_transform = {};
    device_context_->GetTransform(&curr_transform);
    transform = D2D1::Matrix3x2F::Scale(scale, scale)
        * curr_transform
        * D2D1::Matrix3x2F::Translation(originX * ratio, originY * ratio);

    device_context_->SetTransform(transform);

    device_context_->DrawImage(effect.Get());

    device_context_->SetTransform(curr_transform);

    //device_context_->DrawBitmap(bitmap_->Data().Get(), rect);

    return S_OK;
}

HRESULT __stdcall InlineImage::GetMetrics(
    __out DWRITE_INLINE_OBJECT_METRICS* metrics
)
{
    auto bitmap_size = bitmap_->Size();
    auto size = bitmap_size * size_ / bitmap_size.y;
    
    DWRITE_INLINE_OBJECT_METRICS inlineMetrics = {};
    inlineMetrics.width = size.x;
    inlineMetrics.height = size.y;
    inlineMetrics.baseline = size.y * 0.78f;
    *metrics = inlineMetrics;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE InlineImage::GetOverhangMetrics(
    __out DWRITE_OVERHANG_METRICS* overhangs
)
{
    overhangs->left = 0;
    overhangs->top = 0;
    overhangs->right = 0;
    overhangs->bottom = 0;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE InlineImage::GetBreakConditions(
    __out DWRITE_BREAK_CONDITION* breakConditionBefore,
    __out DWRITE_BREAK_CONDITION* breakConditionAfter
)
{
    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;
    return S_OK;
}

ULONG __stdcall InlineImage::AddRef()
{
    return ++nRef_;
}

ULONG __stdcall InlineImage::Release()
{
    if (--nRef_ == 0)
    {
        delete this;
        return 0;
    }
    return nRef_;
}

HRESULT __stdcall InlineImage::QueryInterface(REFIID riid, LPVOID* ppv)
{
    return E_NOINTERFACE;
}
