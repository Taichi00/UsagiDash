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

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float4 color : COLOR;
    uint4 blendIndices : BLENDINDICES;
    float4 blendWeights : BLENDWEIGHTS;
    uint blendNum : BLENDNUM;
};

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
};


VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0; // アウトプット構造体を定義する
    
    float4 localPos = float4(input.pos, 1);
    float4 worldPos = mul(World, localPos);
    float3 viewPos = mul((float3x3) View, worldPos.xyz); // 回転のみ（カメラ座標に固定するため）
    float4 projPos = mul(Proj, float4(viewPos, 1));
    
    output.svpos = projPos;
    output.worldPos = worldPos;
    return output;
}