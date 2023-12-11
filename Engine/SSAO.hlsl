struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

cbuffer Transform : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
}

cbuffer Scene : register(b1)
{
    float4x4 LightView;
    float4x4 LightProj;
    float4 LightColor;
    float3 LightDir;
    float3 CameraPos;
}

Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);
Texture2D gDepth : register(t3);

SamplerState gSampler : register(s0); // ÉTÉìÉvÉâÅ[

static const int SSAO_SAMPLENUM = 16;


float inverseLerp(float a, float b, float value)
{
    return saturate((value - a) / (b - a));
}

float rand(float2 pos, int seed)
{
    float dot_product = dot(seed, pos);
    return frac(sin(dot_product) * 43758.5453);
}

float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    
    float4 color = 0;
    
    float4 texNormal = gNormal.Sample(gSampler, uv);
    float4 texAlbedo = gAlbedo.Sample(gSampler, uv);
    float4 texDepth = gDepth.Sample(gSampler, uv);
    float4 texPosition = gPosition.Sample(gSampler, uv);
    
    float3 normal = normalize(texNormal.xyz);
    float3 albedo = texAlbedo.xyz;
    float shininess = texNormal.w;
    float depth = texDepth.r;
    float outlineMask = texAlbedo.w;
    float3 position = texPosition.xyz;
    
    float2 screenSize;
    gAlbedo.GetDimensions(screenSize.x, screenSize.y);
    
    // SSAO
    float3 samples[SSAO_SAMPLENUM];
    for (int i = 0; i < SSAO_SAMPLENUM; i++)
    {
        float seed = i * 4 + 1;
        samples[i] = float3(rand(uv, seed + 0) * 2 - 1, rand(uv, seed + 1) * 2 - 1, rand(uv, seed + 2));
        samples[i] *= rand(uv, seed + 3);

        float scale = (float) i / SSAO_SAMPLENUM;
        scale = lerp(0.1, 1.0, scale * scale);
        samples[i] *= scale;
    }
    
    
    float3 randomVec = float3(rand(uv, 1), rand(uv, 2), rand(uv, 3));
    float3 viewNormal = mul((float3x3) View, normal);
    float3 viewPosition = mul(View, float4(position, 1)).xyz;
    float3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
    float3 bitangent = cross(viewNormal, tangent);
    float3x3 TBN = float3x3(tangent, bitangent, viewNormal);
    float radius = 0.15;
    float bias = 0.001;
    float occlusion = 0;
    for (int i = 0; i < SSAO_SAMPLENUM; i++)
    {
        float3 sampleVec = mul(samples[i], TBN) * radius;
        float3 samplePos = viewPosition + sampleVec;
        
        float4 offset = mul(Proj, float4(samplePos, 1));
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
        offset.y = -offset.y;

        float sampleDepth = -samplePos.z;
        float realDepth = gDepth.Sample(gSampler, offset.xy).r;
            
        float rangeCheck = smoothstep(0, 1, radius / abs(depth - realDepth));
        occlusion += (realDepth <= sampleDepth - bias ? 1.0 : 0.0) * rangeCheck;
    }
    
    occlusion = 1 - clamp(occlusion / SSAO_SAMPLENUM, 0, 1);
    //occlusion = pow(occlusion, 2);
    color.rgb = occlusion;
    
    return color;
}