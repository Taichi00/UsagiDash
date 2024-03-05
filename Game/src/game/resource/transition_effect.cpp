#include <initguid.h>
#include "transition_effect.h"
#include "util/reader.h"
#include <iostream>

#define XML(X) TEXT(#X)

TransitionEffect::TransitionEffect() : ref_count_(1)
{
    constants_.color = { 0, 0, 0, 0 };
    input_rect_ = {};
}

HRESULT TransitionEffect::UpdateConstants()
{
    return draw_info_->SetPixelShaderConstantBuffer(reinterpret_cast<BYTE*>(&constants_), sizeof(constants_));
}

IFACEMETHODIMP TransitionEffect::Initialize(
    _In_ ID2D1EffectContext* context_internal, 
    _In_ ID2D1TransformGraph* transform_graph)
{
    effect_context_ = context_internal;

    auto reader = Reader();
    std::vector<char> data;

    // シェーダーを読み込む
    try
    {
        data = reader.ReadData(L"assets/shader/transition_ps.cso");
    }
    catch (const char* e)
    {
        std::cout << e << std::endl;
        return E_FAIL;
    }

    auto hr = effect_context_->LoadPixelShader(GUID_TransitionPixelShader, (BYTE*)data.data(), (UINT32)data.size());

    if (SUCCEEDED(hr))
    {
        hr = transform_graph->SetSingleTransformNode(this);
    }

    return hr;
}

IFACEMETHODIMP TransitionEffect::PrepareForRender(D2D1_CHANGE_TYPE change_type)
{
    return UpdateConstants();
}

IFACEMETHODIMP TransitionEffect::SetGraph(_In_ ID2D1TransformGraph* graph)
{
    return E_NOTIMPL;
}

HRESULT TransitionEffect::Register(_In_ ID2D1Factory1* factory)
{
    auto xml =
        L"<?xml version='1.0'?>                                                         "
        L"<Effect>                                                                      "
        L"    <Property name='DisplayName' type='string' value='Transition' />          "
        L"    <Property name='Author' type='string' value='Author' />                   "
        L"    <Property name='Category' type='string' value='Category' />               "
        L"    <Property name='Description' type='string' value='Description' />         "
        L"    <Inputs>                                                                  "
        L"        <Input name='Source' />                                               "
        L"    </Inputs>                                                                 "
        L"    <Property name='Color' type='vector4'>                                    "
        L"        <Property name='DisplayName' type='string' value='Color'/>            "
        L"        <Property name='Default' type='vector4' value='(0.0, 0.0, 0.0, 0.0)'/>"
        L"    </Property>                                                               "
        L"    <Property name='Time' type='float'>                                       "
        L"        <Property name='DisplayName' type='string' value='Time'/>             "
        L"        <Property name='Default' type='float' value='0.0'/>                   "
        L"    </Property>                                                               "
        L"    <Property name='Aspect' type='float'>                                     "
        L"        <Property name='DisplayName' type='string' value='Aspect'/>           "
        L"        <Property name='Default' type='float' value='0.0'/>                   "
        L"    </Property>                                                               "
        L"    <Property name='Direction' type='vector2'>                                "
        L"        <Property name='DisplayName' type='string' value='Direction'/>        "
        L"        <Property name='Default' type='vector2' value='(1.0, 0.0)'/>          "
        L"    </Property>                                                               "
        L"    <Property name='Inverse' type='float'>                                    "
        L"        <Property name='DisplayName' type='string' value='Inverse'/>          "
        L"        <Property name='Default' type='float' value='0.0'/>                   "
        L"    </Property>                                                               "
        L"</Effect>                                                                     "
        ;

    // プロパティのバインド
    const D2D1_PROPERTY_BINDING bindings[] =
    {
        D2D1_VALUE_TYPE_BINDING(L"Color", &SetColor, &GetColor),
        D2D1_VALUE_TYPE_BINDING(L"Time", &SetTime, &GetTime),
        D2D1_VALUE_TYPE_BINDING(L"Aspect", &SetAspect, &GetAspect),
        D2D1_VALUE_TYPE_BINDING(L"Direction", &SetDirection, &GetDirection),
        D2D1_VALUE_TYPE_BINDING(L"Inverse", &SetInverse, &GetInverse),
    };

    auto hr = factory->RegisterEffectFromString(
        CLSID_TransitionEffect,
        xml,
        bindings,
        ARRAYSIZE(bindings),
        CreateEffect
        );

    return hr;
}

HRESULT __stdcall TransitionEffect::CreateEffect(_Outptr_ IUnknown** effect_impl)
{
    *effect_impl = static_cast<ID2D1EffectImpl*>(new (std::nothrow) TransitionEffect());

    if (*effect_impl == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

IFACEMETHODIMP TransitionEffect::SetDrawInfo(_In_ ID2D1DrawInfo* render_info)
{
    draw_info_ = render_info;

    return draw_info_->SetPixelShader(GUID_TransitionPixelShader);
}

IFACEMETHODIMP TransitionEffect::MapInputRectsToOutputRect(
    _In_reads_(input_rect_count) CONST D2D1_RECT_L* input_rects,
    _In_reads_(input_rect_count) CONST D2D1_RECT_L* input_opaque_sub_rects,
    UINT32 input_rect_count,
    _Out_ D2D1_RECT_L* output_rect,
    _Out_ D2D1_RECT_L* output_opaque_sub_rect)
{
    if (input_rect_count != 1)
    {
        return E_INVALIDARG;
    }

    // 境界サイズは変わらない
    *output_rect = input_rects[0];
    input_rect_ = input_rects[0];

    ZeroMemory(output_opaque_sub_rect, sizeof(*output_opaque_sub_rect));
    
    return S_OK;
}

IFACEMETHODIMP TransitionEffect::MapOutputRectToInputRects(
    _In_ const D2D1_RECT_L* output_rect,
    _Out_writes_(input_rect_count) D2D1_RECT_L* input_rects,
    UINT32 input_rect_count) const
{
    if (input_rect_count != 1)
    {
        return E_INVALIDARG;
    }

    input_rects[0] = *output_rect;

    return S_OK;
}

IFACEMETHODIMP TransitionEffect::MapInvalidRect(
    UINT32 input_index,
    D2D1_RECT_L invalid_input_rect,
    _Out_ D2D1_RECT_L* invalid_output_rect) const
{
    if (input_index != 0)
    {
        return E_INVALIDARG;
    }

    *invalid_output_rect = invalid_input_rect;

    return S_OK;
}

UINT32 __stdcall TransitionEffect::GetInputCount() const
{
    return 1;
}

HRESULT TransitionEffect::SetColor(D2D_VECTOR_4F color)
{
    constants_.color = color;
    return S_OK;
}

D2D_VECTOR_4F TransitionEffect::GetColor() const
{
    return constants_.color;
}

HRESULT TransitionEffect::SetTime(float time)
{
    constants_.time = time;
    return S_OK;
}

float TransitionEffect::GetTime() const
{
    return constants_.time;
}

HRESULT TransitionEffect::SetAspect(float aspect)
{
    constants_.aspect = aspect;
    return S_OK;
}

float TransitionEffect::GetAspect() const
{
    return constants_.aspect;
}

HRESULT TransitionEffect::SetDirection(D2D_VECTOR_2F direction)
{
    constants_.direction = direction;
    return S_OK;
}

D2D_VECTOR_2F TransitionEffect::GetDirection() const
{
    return constants_.direction;
}

HRESULT TransitionEffect::SetInverse(float inverse)
{
    constants_.inverse = inverse;
    return S_OK;
}

float TransitionEffect::GetInverse() const
{
    return constants_.inverse;
}

ULONG __stdcall TransitionEffect::AddRef()
{
    ref_count_++;
    return ref_count_;
}

ULONG __stdcall TransitionEffect::Release()
{
    ref_count_--;

    if (ref_count_ == 0)
    {
        delete this;
        return 0;
    }
    else
    {
        return ref_count_;
    }
}

IFACEMETHODIMP TransitionEffect::QueryInterface(_In_ REFIID riid, _Outptr_ void** output)
{
    *output = nullptr;
    HRESULT hr = S_OK;

    if (riid == __uuidof(ID2D1EffectImpl))
    {
        *output = reinterpret_cast<ID2D1EffectImpl*>(this);
    }
    else if (riid == __uuidof(ID2D1DrawTransform))
    {
        *output = static_cast<ID2D1DrawTransform*>(this);
    }
    else if (riid == __uuidof(ID2D1Transform))
    {
        *output = static_cast<ID2D1Transform*>(this);
    }
    else if (riid == __uuidof(ID2D1TransformNode))
    {
        *output = static_cast<ID2D1TransformNode*>(this);
    }
    else if (riid == __uuidof(IUnknown))
    {
        *output = this;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    if (*output != nullptr)
    {
        AddRef();
    }

    return hr;
}
