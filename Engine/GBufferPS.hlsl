struct PSOutput
{
    float4 Position : SV_TARGET0;
    float4 NormalAndSpc : SV_TARGET1;
    float4 AlbedoAndSpcMask : SV_TARGET2;
    float4 Depth : SV_TARGET3;
};

struct PSInput
{
    float4 svpos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
    float4 viewPos : VIEW_POSITION;
    float4 posSM : POSITION_SM;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer MaterialParameter : register(b3)
{
    float4 BaseColor;
    float Shininess;
    float OutlineWidth;
}

SamplerState gSampler : register(s0); // サンプラー
Texture2D gAlbedo : register(t0); // albedoテクスチャ


PSOutput main(PSInput In)
{
    float4 albedo = gAlbedo.Sample(gSampler, In.uv) * float4(BaseColor.rgb, 1);
    
    PSOutput output = (PSOutput) 0;
    output.Position = In.worldPos;
    output.NormalAndSpc.xyz = In.normal.xyz;
    output.NormalAndSpc.w = Shininess;
    output.AlbedoAndSpcMask.xyz = albedo.xyz;
    output.AlbedoAndSpcMask.w = OutlineWidth > 0 ? 0 : 1;
    output.Depth.r = -In.viewPos.z;
    return output;
}