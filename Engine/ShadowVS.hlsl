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

cbuffer BoneParameter : register(b2)
{
    float4x4 boneMatrices[512];
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
    float4 posSM : POSITION_SM;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};


float4 TransformPosition(float4 inPosition, VSInput input)
{
    if (input.blendNum == 0)
        return inPosition;
    
    float4 pos = 0;
    uint indices[4] = (uint[4]) input.blendIndices;
    float weights[4] = (float[4]) input.blendWeights;
    
    for (int i = 0; i < input.blendNum; ++i)
    {
        float4x4 mtx = boneMatrices[indices[i]];
        pos += mul(mtx, inPosition) * weights[i];
    }
    
    pos.w = 1;
    
    return pos;
}

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0; // アウトプット構造体を定義する
    
    float4 localPos = float4(input.pos, 1.0f);
    
    localPos = TransformPosition(localPos, input);
    
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(LightView, worldPos);
    float4 projPos = mul(LightProj, viewPos);
    
    output.svpos = projPos;
    output.posSM = projPos;
    output.normal = input.normal;
    output.color = input.color;
    output.uv = input.uv;
    return output;
}