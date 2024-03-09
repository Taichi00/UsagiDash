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

// RGB->HSV変換
float3 rgb2hsv(float3 rgb)
{
    float3 hsv;

            // RGBの三つの値で最大のもの
    float maxValue = max(rgb.r, max(rgb.g, rgb.b));
            // RGBの三つの値で最小のもの
    float minValue = min(rgb.r, min(rgb.g, rgb.b));
            // 最大値と最小値の差
    float delta = maxValue - minValue;
            
            // V（明度）
            // 一番強い色をV値にする
    hsv.z = maxValue;
            
            // S（彩度）
            // 最大値と最小値の差を正規化して求める
    if (maxValue != 0.0)
    {
        hsv.y = delta / maxValue;
    }
    else
    {
        hsv.y = 0.0;
    }
            
            // H（色相）
            // RGBのうち最大値と最小値の差から求める
    if (hsv.y > 0.0)
    {
        if (rgb.r == maxValue)
        {
            hsv.x = (rgb.g - rgb.b) / delta;
        }
        else if (rgb.g == maxValue)
        {
            hsv.x = 2 + (rgb.b - rgb.r) / delta;
        }
        else
        {
            hsv.x = 4 + (rgb.r - rgb.g) / delta;
        }
        hsv.x /= 6.0;
        if (hsv.x < 0)
        {
            hsv.x += 1.0;
        }
    }
            
    return hsv;
}
        
        // HSV->RGB変換
float3 hsv2rgb(float3 hsv)
{
    float3 rgb;

    if (hsv.y == 0)
    {
                // S（彩度）が0と等しいならば無色もしくは灰色
        rgb.r = rgb.g = rgb.b = hsv.z;
    }
    else
    {
                // 色環のH（色相）の位置とS（彩度）、V（明度）からRGB値を算出する
        hsv.x *= 6.0;
        float i = floor(hsv.x);
        float f = hsv.x - i;
        float aa = hsv.z * (1 - hsv.y);
        float bb = hsv.z * (1 - (hsv.y * f));
        float cc = hsv.z * (1 - (hsv.y * (1 - f)));
        if (i < 1)
        {
            rgb.r = hsv.z;
            rgb.g = cc;
            rgb.b = aa;
        }
        else if (i < 2)
        {
            rgb.r = bb;
            rgb.g = hsv.z;
            rgb.b = aa;
        }
        else if (i < 3)
        {
            rgb.r = aa;
            rgb.g = hsv.z;
            rgb.b = cc;
        }
        else if (i < 4)
        {
            rgb.r = aa;
            rgb.g = bb;
            rgb.b = hsv.z;
        }
        else if (i < 5)
        {
            rgb.r = cc;
            rgb.g = aa;
            rgb.b = hsv.z;
        }
        else
        {
            rgb.r = hsv.z;
            rgb.g = aa;
            rgb.b = bb;
        }
    }
    return rgb;
}

float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    
    float3 color = gTex.Sample(gSampler, uv).rgb;
    
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
    
    //// 露出
    //const float exposure = 1;
    //color = 1.0 - exp(-color * exposure);
    
    // ガンマ
    const float gamma = 2.2;
    color = pow(color, 1.0 / gamma);
    
    // アウトライン
    float outlineNormalWidth = 0.5;
    float3 offset = float3(1.0 / screenSize.x, 1.0 / screenSize.y, 0) * outlineNormalWidth;
    float3 outlineNormal = LaplacianSampleNormal(gNormal, gSampler, uv, offset).rgb;
    float sobelNormal = saturate(outlineNormal.x + outlineNormal.y + outlineNormal.z);
    sobelNormal = step(0.8, sobelNormal);
    float3 outlineColor = lerp(1, 0.2, sobelNormal * outlineMask);
    
    float outlineDepthWidth = 0.5;
    offset = float3(1.0 / screenSize.x, 1.0 / screenSize.y, 0) * outlineDepthWidth;
    float outlineDepth = LaplacianSampleDepth(gDepth, gSampler, uv, offset);
    float sobelDepth = saturate(outlineDepth * 0.3);
    sobelDepth = pow(sobelDepth, 2);
    outlineColor *= lerp(1, 0, sobelDepth * outlineMask);
    
    outlineColor = lerp(0, 1, outlineColor);
    
    float outlinePower = lerp(0.6, 0.85, inverseLerp(30, 80, depth));
    //outlinePower = lerp(outlinePower, 1, inverseLerp(190, 200, depth));
    color = color * lerp(outlinePower, 1, saturate(outlineColor));
    
    // SSAO
    //float ssao = gSSAO.Sample(gSampler, uv).r;
    //color.rgb *= saturate(ssao);
    
    //float contrast = 5.5;
    ////color = 1 / (1 + exp(-contrast * (color - 0.5)));
    
    //float3 hsv = rgb2hsv(color.rgb);
    //hsv.g *= 1.15f;
    //color = hsv2rgb(hsv);
    
    return float4(color, 1);
}
