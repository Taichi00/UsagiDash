struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ

float4 main(VSOutput input) : SV_TARGET
{
    float4 diffuse = _MainTex.Sample(smp, input.uv);
    
    if (diffuse.a == 0.0f)
    {
        discard;
    }
    
    float4 result = float4(diffuse.rgb * 0.5, 1.0f);
    
    return result;
}