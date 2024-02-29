#pragma once

#include <d2d1_3.h>
#include <dwrite_3.h>
#include "game/resource/bitmap.h"
#include "math/vec.h"
#include "math/color.h"

class Engine2D;

struct InlineImage : public IDWriteInlineObject
{
    InlineImage(
        ID2D1DeviceContext* pRenderTarget,
        Bitmap* bitmap,
        float size,
        const Color& color
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
    ID2D1DeviceContext* device_context_ = nullptr;
    Bitmap* bitmap_ = nullptr;
    float size_;
    float baseline_;
    Color color_;

    LONG nRef_ = 0;

    Engine2D* engine_ = nullptr;
};