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

SamplerState smp : register(s0);        // サンプラー
Texture2D _MainTex : register(t0);      // テクスチャ
Texture2D ShadowMap : register(t1);     // シャドウマップ

float4 pixel(VSOutput input) : SV_TARGET
{
    
    float4 diffuse = _MainTex.Sample(smp, input.uv);
    if (diffuse.a == 0.0f)
    {
        discard;
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