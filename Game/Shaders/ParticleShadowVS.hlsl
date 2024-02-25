struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float4 color : COLOR;
    uint InstanceID : SV_InstanceID;
};

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 posSM : POSITION_SM;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct ParticleData
{
    float4x4 world;
    float4x4 lightWorld;
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
    float4x4 LightWorld;
}

cbuffer ParticleParameter : register(b2)
{
    ParticleData data[500];
}

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    // インスタンスIDを取得
    uint index = input.InstanceID;
    float4x4 lightWorld = data[index].lightWorld;
    
    // 座標変換
    float4 localPos = float4(input.pos, 1.0f);
    float4 worldPos = mul(lightWorld, localPos);
    float4 viewPos = mul(LightView, worldPos);
    float4 projPos = mul(LightProj, viewPos);
    
    output.svpos = projPos;
    output.posSM = projPos;
    output.normal = input.normal;
    output.tangent = input.tangent;
    output.color = input.color;
    output.uv = input.uv;
    return output;
}