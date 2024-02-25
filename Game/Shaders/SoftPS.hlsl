struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 posSM : POSITION_SM;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

cbuffer Light : register(b1)
{
    float4x4 LightView;
    float4x4 LightProj;
    float4 LightColor;
    float3 LightDir;
}

SamplerState smp : register(s0); // サンプラー
Texture2D _MainTex : register(t0); // テクスチャ
Texture2D ShadowMap : register(t1); // シャドウマップ



float4 main(VSOutput input) : SV_TARGET
{
    float2 poissonDisk[16] =
    {
        float2(-0.94201624, -0.39906216),
	    float2(0.94558609, -0.76890725),
	    float2(-0.094184101, -0.92938870),
	    float2(0.34495938, 0.29387760),
	    float2(-0.91588581, 0.45771432),
	    float2(-0.81544232, -0.87912464),
	    float2(-0.38277543, 0.27676845),
	    float2(0.97484398, 0.75648379),
	    float2(0.44323325, -0.97511554),
	    float2(0.53742981, -0.47373420),
	    float2(-0.26496911, -0.41893023),
	    float2(0.79197514, 0.19090188),
	    float2(-0.24188840, 0.99706507),
	    float2(-0.81409955, 0.91437590),
	    float2(0.19984126, 0.78641367),
	    float2(0.14383161, -0.14100790)
    };
    
    
    float4 diffuse = _MainTex.Sample(smp, input.uv);
    if (diffuse.a == 0.0f)
    {
        discard;
    }
    
    // 陰
    float p = dot(input.normal * -1.0f, LightDir);
    p = p * 0.5f + 0.5f;
    p = (float) round(p);
    
    // シャドウマップ
    float4 possm = input.posSM;
    if (possm.x >= 0 && possm.x <= 1 && possm.y >= 0 && possm.y <= 1)
    {
        float sum = 0.0f;
        float distance = possm.z / possm.w;
        for (int i = 0; i < 16; i++)
        {
            float2 offset = poissonDisk[i] * 0.8 / 1024;
            float sm = ShadowMap.Sample(smp, saturate(possm.xy + offset)).r;
            if (distance - 0.002f < sm)
            {
                sum += 1.0f;
            }
        }
        p = min(p, sum / 16.0f);
        
    }
   
    float4 color = float4(0.8f, 0.75f, 0.78f, 1.0f);
    //color = float4(0.5f, 0.5f, 0.5f, 1.0f);
    color.rgb += p * (float3(1.0f, 1.0f, 1.0f) - color.rgb);
    
    float4 ret = float4(diffuse.rgb * color.rgb, diffuse.a);
    
    return ret;
}