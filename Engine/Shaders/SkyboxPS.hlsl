struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
};

SamplerState gSampler : register(s0); // サンプラー
TextureCube gSkybox : register(t0); // テクスチャ

float4 main(VSOutput input) : SV_TARGET
{
    float3 envColor = gSkybox.SampleLevel(gSampler, normalize(input.worldPos.xyz), 0).rgb;
    
    // ガンマ補正
    envColor = envColor / (envColor + 1.0);
    envColor = pow(envColor, 1.0 / 2.2);
    
    return float4(envColor, 1.0);
}