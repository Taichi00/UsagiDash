#pragma once

#include "engine/comptr.h"
#include <basetsd.h>
#include <d2d1_3.h>
#include <d2d1effectauthor.h>
#include <d2d1effecthelpers.h>
#include <guiddef.h>

DEFINE_GUID(CLSID_TransitionEffect,
    0x108496da, 0x4712, 0x42b0, 0xa9, 0x91, 0xd4, 0xa1, 0x9e, 0xf1, 0xbd, 0xec);

DEFINE_GUID(GUID_TransitionPixelShader,
    0x7e8c77dc, 0x44e1, 0x4152, 0xa0, 0x45, 0xc4, 0x2, 0x99, 0x59, 0xab, 0x62);

enum TRANSITION_PROP
{
    TRANSITION_PROP_COLOR,
    TRANSITION_PROP_TIME,
    TRANSITION_PROP_ASPECT,
    TRANSITION_PROP_DIRECTION,
    TRANSITION_PROP_INVERSE,
};

class TransitionEffect : public ID2D1EffectImpl, public ID2D1DrawTransform
{
public:
    // èâä˙âª
	IFACEMETHODIMP Initialize(
		_In_ ID2D1EffectContext* context_internal,
		_In_ ID2D1TransformGraph* transform_graph
	);

    IFACEMETHODIMP PrepareForRender(D2D1_CHANGE_TYPE change_type);
    IFACEMETHODIMP SetGraph(_In_ ID2D1TransformGraph* graph);

    static HRESULT Register(_In_ ID2D1Factory1* factory);
    static HRESULT __stdcall CreateEffect(_Outptr_ IUnknown** effect_impl);

    IFACEMETHODIMP SetDrawInfo(_In_ ID2D1DrawInfo* render_info);

    // èoóÕÉCÉÅÅ[ÉWÇÃã´äEÇåvéZÇ∑ÇÈ
    IFACEMETHODIMP MapInputRectsToOutputRect(
        _In_reads_(input_rect_count) CONST D2D1_RECT_L* input_rects,
        _In_reads_(input_rect_count) CONST D2D1_RECT_L* input_opaque_sub_rects,
        UINT32 input_rect_count,
        _Out_ D2D1_RECT_L* output_rect,
        _Out_ D2D1_RECT_L* output_opaque_sub_rect
    );

    IFACEMETHODIMP MapOutputRectToInputRects(
        _In_ const D2D1_RECT_L* output_rect,
        _Out_writes_(input_rect_count) D2D1_RECT_L* input_rects,
        UINT32 input_rect_count
    ) const;

    // ïœä∑ÇÃñ≥å¯Ç»ã´äEÇåvéZÇ∑ÇÈ
    IFACEMETHODIMP MapInvalidRect(
        UINT32 input_index,
        D2D1_RECT_L invalid_input_rect,
        _Out_ D2D1_RECT_L* invalid_output_rect
    ) const;

    // Transform ÇÃì¸óÕêîÇï‘Ç∑
    IFACEMETHODIMP_(UINT32) GetInputCount() const;

    HRESULT SetColor(D2D_VECTOR_4F color);
    D2D_VECTOR_4F GetColor() const;

    HRESULT SetTime(float time);
    float GetTime() const;

    HRESULT SetAspect(float aspect);
    float GetAspect() const;

    HRESULT SetDirection(D2D_VECTOR_2F direction);
    D2D_VECTOR_2F GetDirection() const;

    HRESULT SetInverse(float inverse);
    float GetInverse() const;

    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();
    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void** output);

private:
    TransitionEffect();
    HRESULT UpdateConstants();

private:
    struct
    {
        D2D_VECTOR_4F color;
        float time;
        float aspect;
        D2D_VECTOR_2F direction;
        float inverse;
    } constants_;

    LONG ref_count_;

    ComPtr<ID2D1DrawInfo> draw_info_;
    ComPtr<ID2D1EffectContext> effect_context_;
    D2D1_RECT_L input_rect_;

};