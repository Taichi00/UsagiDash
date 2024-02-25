struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
    float4 viewPos : VIEW_POSITION;
    float4 posSM : POSITION_SM;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
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
Texture2D ShadowMap : register(t1); // シャドウマップ

float4 main(VSOutput input) : SV_TARGET
{
    
    float4 diffuse = _MainTex.Sample(smp, input.uv) * BaseColor;
    if (diffuse.a == 0.0f)
    {
        discard;
    }
    
    float p = dot(input.normal * -1.0f, LightDir);
    p = p * 0.5f + 0.5f;
    p = (float) round(p);
    
    float sm = ShadowMap.Sample(smp, input.posSM.xy).r;
    p = (input.posSM.z - 0.005f < sm) ? p : 0.5f;
    
    float4 color = float4(0.9f, 0.78f, 0.85f, 1.0f);
    //color = float4(0, 0, 0, 1.0f);
    color.rgb += p * (float3(1.0f, 1.0f, 1.0f) - color.rgb);
    
    float4 ret = float4(diffuse.rgb * color.rgb, diffuse.a);
    
    return ret;
}