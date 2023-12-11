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

Texture2D gTex : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);
Texture2D gDepth : register(t3);
Texture2D gPosition : register(t4);
Texture2D gSSAO : register(t5);

SamplerState gSampler : register(s0); // サンプラー

static const int SSAO_SAMPLENUM = 10;


float inverseLerp(float a, float b, float value)
{
    return saturate((value - a) / (b - a));
}

float rand(float2 pos, int seed)
{
    float dot_product = dot(seed, pos);
    return frac(sin(dot_product) * 43758.5453);
}

float3 SobelSampleNormal(Texture2D t, SamplerState s, float2 uv, float3 offset)
{
    float3 pixelCenter = t.Sample(s, uv).rgb;
    float3 pixelLeft = t.Sample(s, uv - offset.xz).rgb;
    float3 pixelRight = t.Sample(s, uv + offset.xz).rgb;
    float3 pixelUp = t.Sample(s, uv + offset.zy).rgb;
    float3 pixelDown = t.Sample(s, uv - offset.zy).rgb;
    
    return abs(pixelLeft - pixelCenter) +
           abs(pixelRight - pixelCenter) +
           abs(pixelUp - pixelCenter) +
           abs(pixelDown - pixelCenter);
}

float SobelSampleDepth(Texture2D t, SamplerState s, float2 uv, float3 offset)
{
    float pixelCenter = t.Sample(s, uv).r;
    float pixelLeft = t.Sample(s, uv - offset.xz).r;
    float pixelRight = t.Sample(s, uv + offset.xz).r;
    float pixelUp = t.Sample(s, uv + offset.zy).r;
    float pixelDown = t.Sample(s, uv - offset.zy).r;
    
    return abs(pixelLeft - pixelCenter) +
           abs(pixelRight - pixelCenter) +
           abs(pixelUp - pixelCenter) +
           abs(pixelDown - pixelCenter);
}

float3 LaplacianSampleNormal(Texture2D t, SamplerState s, float2 uv, float3 offset)
{
    float2 off[9];
    off[0] = float2(-1.0, -1.0) * offset.xy;
    off[1] = float2( 0.0, -1.0) * offset.xy;
    off[2] = float2( 1.0, -1.0) * offset.xy;
    off[3] = float2(-1.0,  0.0) * offset.xy;
    off[4] = float2( 0.0,  0.0) * offset.xy;
    off[5] = float2( 1.0,  0.0) * offset.xy;
    off[6] = float2(-1.0,  1.0) * offset.xy;
    off[7] = float2( 0.0,  1.0) * offset.xy;
    off[8] = float2( 1.0,  1.0) * offset.xy;
    
    //float coef[9] =
    //{
    //    1.0, 1.0, 1.0,
    //    1.0, -8.0, 1.0,
    //    1.0, 1.0, 1.0
    //};
    float coef[9] =
    {
        0.0, 1.0, 0.0,
        1.0, -4.0, 1.0,
        0.0, 1.0, 0.0
    };
    
    float3 color = 0;
    color += t.Sample(s, uv + off[0]).rgb * coef[0];
    color += t.Sample(s, uv + off[1]).rgb * coef[1];
    color += t.Sample(s, uv + off[2]).rgb * coef[2];
    color += t.Sample(s, uv + off[3]).rgb * coef[3];
    color += t.Sample(s, uv + off[4]).rgb * coef[4];
    color += t.Sample(s, uv + off[5]).rgb * coef[5];
    color += t.Sample(s, uv + off[6]).rgb * coef[6];
    color += t.Sample(s, uv + off[7]).rgb * coef[7];
    color += t.Sample(s, uv + off[8]).rgb * coef[8];
    
    return abs(color);
}

float3 LaplacianSampleDepth(Texture2D t, SamplerState s, float2 uv, float3 offset)
{
    float2 off[9];
    off[0] = float2(-1.0, -1.0) * offset.xy;
    off[1] = float2(0.0, -1.0) * offset.xy;
    off[2] = float2(1.0, -1.0) * offset.xy;
    off[3] = float2(-1.0, 0.0) * offset.xy;
    off[4] = float2(0.0, 0.0) * offset.xy;
    off[5] = float2(1.0, 0.0) * offset.xy;
    off[6] = float2(-1.0, 1.0) * offset.xy;
    off[7] = float2(0.0, 1.0) * offset.xy;
    off[8] = float2(1.0, 1.0) * offset.xy;
    
    //float coef[9] =
    //{
    //    1.0, 1.0, 1.0,
    //    1.0, -8.0, 1.0,
    //    1.0, 1.0, 1.0
    //};
    float coef[9] =
    {
        0.0, 1.0, 0.0,
        1.0, -4.0, 1.0,
        0.0, 1.0, 0.0
    };
    
    float color = 0;
    color += t.Sample(s, uv + off[0]).r * coef[0];
    color += t.Sample(s, uv + off[1]).r * coef[1];
    color += t.Sample(s, uv + off[2]).r * coef[2];
    color += t.Sample(s, uv + off[3]).r * coef[3];
    color += t.Sample(s, uv + off[4]).r * coef[4];
    color += t.Sample(s, uv + off[5]).r * coef[5];
    color += t.Sample(s, uv + off[6]).r * coef[6];
    color += t.Sample(s, uv + off[7]).r * coef[7];
    color += t.Sample(s, uv + off[8]).r * coef[8];
    
    return abs(color);
}


float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    
    float4 color = gTex.Sample(gSampler, uv);
    
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
        
    // アウトライン
    float outlineNormalWidth = 0.5;
    float3 offset = float3(1.0 / screenSize.x, 1.0 / screenSize.y, 0) * outlineNormalWidth;
    float3 outlineNormal = LaplacianSampleNormal(gNormal, gSampler, uv, offset).rgb;
    float sobelNormal = saturate(outlineNormal.x + outlineNormal.y + outlineNormal.z);
    //sobelNormal = pow(sobelNormal, 2);
    sobelNormal = step(0.8, sobelNormal);
    //sobelNormal = inverseLerp(0.5, 1, sobelNormal);
    float3 outlineColor = lerp(1, 0.4, sobelNormal * outlineMask);
    
    float outlineDepthWidth = 0.5;
    offset = float3(1.0 / screenSize.x, 1.0 / screenSize.y, 0) * outlineDepthWidth;
    float outlineDepth = LaplacianSampleDepth(gDepth, gSampler, uv, offset);
    float sobelDepth = saturate(outlineDepth * 0.3);
    sobelDepth = pow(sobelDepth, 2);
    //sobelDepth = step(0.999, sobelDepth);
    //sobelDepth = inverseLerp(0.5, 1, sobelDepth);
    outlineColor *= lerp(1, 0, sobelDepth * outlineMask);
    
    outlineColor = lerp(float3(0.0, 0.0, 0.0), 1, outlineColor);
    
    float outlinePower = lerp(0.1, 0.6, inverseLerp(30, 80, depth));
    color.rgb = color.rgb * lerp(outlinePower, 1, saturate(outlineColor));
    //color.rgb = saturate(length(outlineNormal.x + outlineNormal.y + outlineNormal.z));
    
    // SSAO
    float ssao = gSSAO.Sample(gSampler, uv).r;
    ssao = pow(ssao, 1.5);
    color.rgb *= lerp(float3(0, 0, 0.5), 1, ssao);
    color.rgb *= ssao;
    
    return color;
}
