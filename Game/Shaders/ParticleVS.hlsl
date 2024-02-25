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
    float4 worldPos : WORLD_POSITION;
    float4 viewPos : VIEW_POSITION;
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
}

cbuffer ParticleParameter : register(b2)
{
    ParticleData data[500];
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
    VSOutput output = (VSOutput) 0;
    
    // インスタンスIDを取得
    uint index = input.InstanceID;
    float4x4 world = data[index].world;
    
    // 座標変換
    float4 localPos = float4(input.pos, 1);
    float4 worldPos = mul(world, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);
   
    float3 localNormal = input.normal;
    float3 worldNormal = mul((float3x3) world, localNormal);
    worldNormal = normalize(worldNormal);
    
    float3 localTangent = input.tangent;
    float3 worldTangent = mul((float3x3) world, localTangent);
    worldTangent = normalize(worldTangent);
    
    float3 localBinormal = cross(input.normal, input.tangent);
    float3 worldBinormal = mul((float3x3) world, localBinormal);
    worldBinormal = normalize(worldBinormal);
    
    // ShadowMap上の座標を取得
    float4 possm = GetShadowMapPosition(worldPos, input);
    
    
    output.svpos = projPos;
    output.worldPos = worldPos;
    output.viewPos = viewPos;
    output.posSM = possm;
    output.normal = worldNormal;
    output.tangent = worldTangent;
    output.binormal = worldBinormal;
    output.color = input.color;
    output.uv = input.uv;
    return output;
}