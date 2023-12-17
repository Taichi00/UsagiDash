struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
    float4 viewPos : VIEW_POSITION;
    float4 posSM : POSITION_SM;
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

cbuffer MaterialParameter : register(b3)
{
    float4 BaseColor;
    float Shininess;
    float OutlineWidth;
}

static const int pattern[16] =
{
     0,  8,  2, 10,
    12,  4, 14,  6,
     3, 11,  1,  9,
    15,  7, 13,  5
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
    //float2 screenSize = float2(1280, 720);
    
    if (OutlineWidth > 0)
    {
        float2 screenPos = input.svpos.xy / input.viewPos.w;
        float ditherLevel = inverseLerp(-5, -2, input.svpos.z) * 16;
        DitherClip(screenPos, DitherLevel, 2);
    }
    
    return 1;
}