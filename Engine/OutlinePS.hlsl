struct VSOutput
{
    float4 svpos : SV_POSITION;
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

cbuffer MaterialParameter : register(b3)
{
    float4 BaseColor;
    float Shininess;
    float OutlineWidth;
}

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ


float inverseLerp(float a, float b, float value)
{
    return saturate((value - a) / (b - a));
}

float4 main(VSOutput input) : SV_TARGET
{
    float4 diffuse = _MainTex.Sample(smp, input.uv) * BaseColor;
    
    if (diffuse.a == 0.0f)
    {
        discard;
    }
    
    float outlinePower = 1 - (dot(normalize(input.normal), LightDir) * 0.5 + 0.5);
    outlinePower = lerp(0.1, 0.5, inverseLerp(0.6, 1, outlinePower));
    float3 outlineColor = diffuse.rgb * outlinePower;
    
    float4 result = float4(outlineColor, 1.0f);
    
    return result;
}