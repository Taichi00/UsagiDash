struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
};

SamplerState gSampler : register(s0); // サンプラー
TextureCube gSkybox : register(t0); // テクスチャ

float4 main(VSOutput input) : SV_TARGET
{
    return gSkybox.Sample(gSampler, normalize(input.worldPos.xyz));
    //float4 c = gSkybox.Sample(gSampler, normalize(input.worldPos.xyz));
    //return float4(1, 0, 0, 1);
}