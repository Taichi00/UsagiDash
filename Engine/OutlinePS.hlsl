struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 viewPos : VIEW_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer Transform : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
    float DitherLevel;
}

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


static const int pattern[16] =
{
    0, 8, 2, 10,
    12, 4, 14, 6,
     3, 11, 1, 9,
    15, 7, 13, 5
};

float inverseLerp(float a, float b, float value)
{
    return saturate((value - a) / (b - a));
}

void DitherClip(float2 screenPos, float ditherLevel, float size)
{
    int ditherUV_x = (int) fmod(screenPos.x / size, 4.0f);
    int ditherUV_y = (int) fmod(screenPos.y / size, 4.0f);
    float dither = pattern[ditherUV_x + ditherUV_y * 4];
    
    clip(dither - ditherLevel);
}

float4 main(VSOutput input) : SV_TARGET
{
    float4 diffuse = _MainTex.Sample(smp, input.uv) * BaseColor;
    
    if (diffuse.a == 0.0f)
    {
        discard;
    }
    
    // ディザリング
    float2 screenPos = input.svpos.xy / input.viewPos.w;
    float ditherLevel = inverseLerp(-5, -2, input.viewPos.z) * 16;
    DitherClip(screenPos, DitherLevel, 1);
    
    // カラーリング
    float outlinePower = 1 - (dot(normalize(input.normal), LightDir) * 0.5 + 0.5);
    outlinePower = lerp(0.1, 0.5, inverseLerp(0.6, 1, outlinePower));
    float3 outlineColor = diffuse.rgb * outlinePower;
    
    float4 result = float4(outlineColor, 1.0f);
    
    return result;
}