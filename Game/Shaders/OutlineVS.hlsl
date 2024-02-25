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

cbuffer MaterialParameter : register(b3)
{
    float4 BaseColor;
    float Shininess;
    float OutlineWidth;
}

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : SMOOTHNORMAL;
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
    float4 viewPos : VIEW_POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
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
    
    for (uint i = 0; i < input.blendNum; ++i)
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
    
    for (uint i = 0; i < input.blendNum; ++i)
    {
        float4x4 mtx = boneMatrices[indices[i]];
        normal += mul((float3x3) mtx, inNormal) * weights[i];
    }
        
    return normal;
}


VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput) 0; // アウトプット構造体を定義する
    
    float4 localPos = TransformPosition(float4(input.pos, 1.0f), input);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);
    
    float3 localNormal = TransformNormal(input.normal, input);
    float3 worldNormal = mul((float3x3) World, localNormal);
    float3 viewNormal = mul((float3x3) View, worldNormal);
    float3 projNormal = mul((float3x3) Proj, normalize(viewNormal));
    float2 offset = projNormal.xy;
    
    output.svpos = projPos;
    output.svpos.xy += projNormal.xy * OutlineWidth * projPos.w;
    
    output.viewPos = viewPos;
    output.normal = worldNormal;
    output.color = input.color;
    output.uv = input.uv;
    return output;
}