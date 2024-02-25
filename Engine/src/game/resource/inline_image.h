#pragma once

#include <d2d1_3.h>
#include <dwrite_3.h>
#include "game/resource/bitmap.h"
#include "math/vec.h"

struct InlineImage : public IDWriteInlineObject
{
    InlineImage(
        ID2D1RenderTarget* pRenderTarget,
        Bitmap* bitmap,
        float size
    );

    HRESULT STDMETHODCALLTYPE Draw(
        __maybenull void* clientDrawingContext,
        IDWriteTextRenderer* renderer,
        FLOAT originX,
        FLOAT originY,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
    ) override;

    HRESULT STDMETHODCALLTYPE GetMetrics(
        __out DWRITE_INLINE_OBJECT_METRICS* metrics
    ) override;

    HRESULT STDMETHODCALLTYPE GetOverhangMetrics(
        __out DWRITE_OVERHANG_METRICS* overhangs
    ) override;

    HRESULT STDMETHODCALLTYPE GetBreakConditions(
        __out DWRITE_BREAK_CONDITION* breakConditionBefore,
        __out DWRITE_BREAK_CONDITION* breakConditionAfter
    ) override;

    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID* ppv) override;

private:
    ID2D1RenderTarget* pRT_ = nullptr;
    Bitmap* bitmap_ = nullptr;
    Vec2 size_;
    float baseline_;

    LONG nRef_ = 0;
};