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
    uint4  blendIndices : BLENDINDICES;
    float4 blendWeights : BLENDWEIGHTS;
    uint blendNum : BLENDNUM;
};

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 worldPos : WORLD_POSITION;
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

float3 TransformNormal(float3 inNormal, VSInput input)
{
    if (input.blendNum == 0)
        return inNormal;
    
    float3 normal = 0;
    uint indices[4] = (uint[4]) input.blendIndices;
    float weights[4] = (float[4]) input.blendWeights;
    
    if (weights[0] == 0)
        return inNormal;
    
    for (int i = 0; i < input.blendNum; ++i)
    {
        float4x4 mtx = boneMatrices[indices[i]];
        normal += mul((float3x3) mtx, inNormal) * weights[i];
    }
        
    return normal;
}

float4 GetShadowMapPosition(float4 worldPos, VSInput input)
{
    float4 posSM = 0;
    
    float4 smPos = mul(LightView, worldPos);
    smPos = mul(LightProj, smPos);
    posSM.x = (1.0f + smPos.x) / 2.0f;
    posSM.y = (1.0f - smPos.y) / 2.0f;
    posSM.z = smPos.z;
    posSM.w = smPos.w;
    
    return posSM;
}

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0; // �A�E�g�v�b�g�\���̂��`����
    
    float4 localPos = TransformPosition(float4(input.pos, 1), input);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);
    
    float3 localNormal = TransformNormal(input.normal, input);
    float3 worldNormal = mul((float3x3) World, localNormal);
    worldNormal = normalize(worldNormal);
    
    output.svpos = projPos;
    output.worldPos = worldPos;
    output.posSM = GetShadowMapPosition(worldPos, input);
    output.normal = worldNormal;
    output.color = input.color;
    output.uv = input.uv;
    return output;
}