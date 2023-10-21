struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 posSM : POSITION_SM;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer Scene : register(b1)
{
    float4x4 LightView;
    float4x4 LightProj;
    float4 LightColor;
    float3 LightDir;
    float3 CameraPos;
}

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

float4 main(VSOutput input) : SV_TARGET
{
    float4 diffuse = _MainTex.Sample(smp, input.uv);
    
    if (diffuse.a == 0.0f)
    {
        discard;
    }
    
    float d = input.posSM.z / input.posSM.w;
    
    return float4(d, d * d, d, 1);
}