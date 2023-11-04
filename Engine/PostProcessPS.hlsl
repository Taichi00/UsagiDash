struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

cbuffer Scene : register(b0)
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

SamplerState gSampler : register(s0); // サンプラー


float inverseLerp(float a, float b, float value)
{
    return saturate((value - a) / (b - a));
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


float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    
    float4 color = gTex.Sample(gSampler, uv);
    
    float4 texNormal = gNormal.Sample(gSampler, uv);
    float4 texAlbedo = gAlbedo.Sample(gSampler, uv);
    float4 texDepth = gDepth.Sample(gSampler, uv);
    
    float3 normal = normalize(texNormal.xyz);
    float3 albedo = texAlbedo.xyz;
    float shininess = texNormal.w;
    float depth = texDepth.r;
    float outlineMask = texAlbedo.w;
    
    float2 screenSize;
    gAlbedo.GetDimensions(screenSize.x, screenSize.y);
        
    // アウトライン
    float outlineNormalWidth = 2;
    float3 offset = float3(1.0 / screenSize.x, 1.0 / screenSize.y, 0) * outlineNormalWidth;
    float3 outlineNormal = SobelSampleNormal(gNormal, gSampler, uv, offset).rgb;
    float sobelNormal = saturate(length(outlineNormal.x + outlineNormal.y + outlineNormal.z) * 0.3);
    //sobelNormal = pow(sobelNormal, 2);
    //sobelNormal = step(0.9, sobelNormal);
    sobelNormal = inverseLerp(0.5, 1, sobelNormal);
    float3 outlineColor = lerp(1, 0, sobelNormal * outlineMask);
    
    float outlineDepthWidth = 1;
    offset = float3(1.0 / screenSize.x, 1.0 / screenSize.y, 0) * outlineDepthWidth;
    float outlineDepth = SobelSampleDepth(gDepth, gSampler, uv, offset);
    float sobelDepth = saturate(outlineDepth * 0.2);
    //sobelDepth = pow(sobelDepth, 2);
    //sobelDepth = step(0.999, sobelDepth);
    //sobelDepth = inverseLerp(0.5, 1, sobelDepth);
    outlineColor *= lerp(1, 0, sobelDepth * outlineMask);
    
    float outlinePower = saturate(inverseLerp(50, 80, depth));
    color.rgb = color.rgb * lerp(0.1, 1, outlineColor);
    
    return color;
}