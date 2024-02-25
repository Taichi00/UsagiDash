#include "inline_image.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "game/game.h"

InlineImage::InlineImage(ID2D1RenderTarget* pRenderTarget, Bitmap* bitmap, float size)
{
    pRT_ = pRenderTarget;
    pRT_->AddRef();

    bitmap_ = bitmap;
    
    auto bitmap_size = bitmap_->Size();
    float ratio = size / bitmap_size.y;
    size_ = bitmap_size * ratio;

    baseline_ = size_.y * 0.75f;
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
    D2D1_RECT_F rect = {originX, originY, originX + size_.x, originY + size_.y};

    pRT_->DrawBitmap(bitmap_->Data().Get(), rect);

    return S_OK;
}

HRESULT __stdcall InlineImage::GetMetrics(
    __out DWRITE_INLINE_OBJECT_METRICS* metrics
)
{
    DWRITE_INLINE_OBJECT_METRICS inlineMetrics = {};
    inlineMetrics.width = size_.x;
    inlineMetrics.height = size_.y;
    inlineMetrics.baseline = baseline_;
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
