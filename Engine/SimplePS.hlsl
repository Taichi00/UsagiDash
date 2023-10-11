struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 posSM : POSITION_SM;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer Light : register(b1)
{
    float4x4 LightView;
    float4x4 LightProj;
    float4 LightColor;
    float3 LightDir;
}

cbuffer MaterialParameter : register(b3)
{
    float4 BaseColor;
    float OutlineWidth;
}

SamplerState smp : register(s0);        // サンプラー
Texture2D _MainTex : register(t0);      // テクスチャ
Texture2D ShadowMap : register(t1);     // シャドウマップ


float hash(float2 input)
{
    return frac(1.0e4 * sin(17.0 * input.x + 0.1 * input.y) * (0.1 + abs(sin(13.0 * input.y + input.x))));
}

float hash3D(float3 input)
{
    return hash(float2(hash(input.xy), input.z));
}

float hashedAlpha(float4 objCoord)
{
    float g_HashScale = 1.0;

    float maxDeriv = max(length(ddx(objCoord.xyz)), length(ddy(objCoord.xyz)));
    float pixScale = 1.0 / (g_HashScale * maxDeriv);
    
    float2 pixScales = float2(exp2(floor(log2(pixScale))), exp2(ceil(log2(pixScale))));
    
    float2 alpha = float2(hash3D(floor(pixScales.x * objCoord.xyz)), hash3D(floor(pixScales.y * objCoord.xyz)));

    float lerpFactor = frac(log2(pixScale));
    
    float x = (1 - lerpFactor) * alpha.x + lerpFactor * alpha.y;
    
    float a = min(lerpFactor, 1 - lerpFactor);
    float3 cases = float3(x * x / (2 * a * (1 - a)), (x - 0.5 * a) / (1 - a), 1.0 - ((1 - x) * (1 - x) / (2 * a * (1 - a))));

    float res = (x < (1 - a)) ? ((x < a) ? cases.x : cases.y) : cases.z;

    return clamp(res, 1.0e-6, 1.0);
}

float4 pixel(VSOutput input) : SV_TARGET
{
    
    float4 diffuse = _MainTex.Sample(smp, input.uv) * BaseColor;
    if (diffuse.a < 1.0)
    {
        if (diffuse.a < hashedAlpha(input.svpos))
        {
            discard;
        }
    }
    
    // 陰
    float p = dot(input.normal * -1.0f, LightDir);
    p = p * 0.5f + 0.5f;
    p = (float) round(p);
    
    // シャドウマップ
    float4 possm = input.posSM;
    if (possm.x >= 0 && possm.x <= 1 && possm.y >= 0 && possm.y <= 1)
    {
        float sm = ShadowMap.Sample(smp, possm.xy).r;
        p = (possm.z / possm.w - 0.002f < sm) ? p : 0.0f;
    }
   
    float4 color = float4(0.8f, 0.75f, 0.78f, 1.0f);
    //color = float4(0.5f, 0.5f, 0.5f, 1.0f);
    color.rgb += p * (float3(1.0f, 1.0f, 1.0f) - color.rgb);
    
    float4 ret = float4(diffuse.rgb * color.rgb, diffuse.a);
    
    return ret;
}