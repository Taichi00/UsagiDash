struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

cbuffer Scene : register(b0)
{
    float4x4 LightView;
    float4x4 LightProj;
    float4 LightColor;
    float3 LightDir;
    float3 CameraPos;
}

Texture2D gPosition : register(t0);
Texture2D gNormal : register(t1);
Texture2D gAlbedo : register(t2);
Texture2D gMetallicRoughness : register(t3);
Texture2D gDepth : register(t4);
Texture2D gShadowMap : register(t5);
TextureCube gDiffuseMap : register(t6);
TextureCube gSpecularMap : register(t7);
Texture2D gBrdfLUT : register(t8);

SamplerState gSampler : register(s0);


static const float PI = 3.14159265359;
static const float2 poissonDisk[16] =
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


float inverseLerp(float a, float b, float value)
{
    return saturate((value - a) / (b - a));
}

// 乱数を生成する
float rand(float2 pos, int seed)
{
    float dot_product = dot(seed, pos);
    return frac(sin(dot_product) * 43758.5453);
}

float linstep(float min, float max, float v)
{
    return clamp((v - min) / (max - min), 0, 1);
}

float ReduceLightBleeding(float p_max, float Amount)
{
    return linstep(Amount, 1, p_max);
}

// 分散シャドウマップ
float VarianceShadow(float4 possm)
{
    float w = 1.0f / possm.w;
    float2 stex = possm.xy;
    
    float2 depth = gShadowMap.Sample(gSampler, stex.xy).xy;
    
    float depth_sq = depth.x * depth.x;
    float variance = depth.y - depth_sq;
    variance = min(1.0, max(0.0, variance + 0.0000002));
    
    float fragDepth = possm.z * w;
    float md = fragDepth - depth.x;
    float p = variance / (variance + (md * md));
    
    p = max(p, fragDepth <= depth.x);
    p = ReduceLightBleeding(p, 0.8);
    
    // 境界をぼかす
    float border = 1 - inverseLerp(0, 0.1, possm.x) * inverseLerp(1, 0.9, possm.x) * inverseLerp(0, 0.1, possm.y) * inverseLerp(1, 0.9, possm.y);
    
    return saturate(p + border);
}

// ポアソンシャドウ
float PoissonShadow(float4 possm, int n, float size, float2 uv)
{
    float sum = 0;
    float distance = possm.z / possm.w;
    for (int i = 0; i < n; i++)
    {
        int index = int(16.0 * rand(uv, i)) % 16;
        float2 offset = poissonDisk[index] * size / 1024;
        float sm = gShadowMap.Sample(gSampler, saturate(possm.xy + offset)).r;
        if (distance < sm + 0.0009)
        {
            sum += 1;
        }
    }
    
    // 境界をぼかす
    float border = 1 - inverseLerp(0, 0.1, possm.x) * inverseLerp(1, 0.9, possm.x) * inverseLerp(0, 0.1, possm.y) * inverseLerp(1, 0.9, possm.y);
    
    return saturate(sum / n + border);
}

float Shadow(float4 possm)
{
    float shadow = 0;
    float distance = possm.z / possm.w;
    float sm = gShadowMap.Sample(gSampler, possm.xy).r;
    if (distance < sm + 0.0001)
    {
        shadow = 1;
    }
    
    // 境界をぼかす
    float border = 1 - inverseLerp(0, 0.1, possm.x) * inverseLerp(1, 0.9, possm.x) * inverseLerp(0, 0.1, possm.y) * inverseLerp(1, 0.9, possm.y);
    
    return saturate(shadow + border);
}

// シャドウマップ上の座標を返す
float4 GetShadowMapPosition(float4 worldPos)
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

// 正規分布関数
// ハイライト部分を計算する
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

// ジオメトリ関数
// 表面が粗いほど陰がぼける
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// 視線ベクトルと光方向ベクトルの両方を考慮してジオメトリを近似する
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// フレネル反射
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// フレネル反射（roughnessを考慮）
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(1.0 - roughness, F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Returns number of mipmap levels for specular IBL environment map.
uint querySpecularTextureLevels()
{
    uint width, height, levels;
    gSpecularMap.GetDimensions(0, width, height, levels);
    return levels;
}


float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    
    float4 texPosition = gPosition.Sample(gSampler, uv);
    float4 texNormal = gNormal.Sample(gSampler, uv);
    float4 texAlbedo = gAlbedo.Sample(gSampler, uv);
    float4 texMetallicRoughness = gMetallicRoughness.Sample(gSampler, uv);
    float4 texDepth = gDepth.Sample(gSampler, uv);
    
    float4 worldPos = texPosition.xyzw;
    float3 normal = normalize(texNormal.xyz);
    float3 albedo = pow(texAlbedo.xyz, 2.2);
    float metallic = texMetallicRoughness.b;
    float roughness = texMetallicRoughness.g;
    //float ao = texMetallicRoughness.r;
    float shininess = texNormal.w;
    float depth = texDepth.r;
    
    // パラメータ（ConstantBufferにする）
    //metallic = 0;
    //roughness = 0.1;
    float ao = 1;
    float spec = 0.129;
    
    float3 N = normalize(normal); // 法線ベクトル
    float3 V = normalize(CameraPos - worldPos.xyz); // 視線ベクトル
    float3 R = reflect(-V, N); // 反射ベクトル
    
    // 反射率を表すF0を求める
    float3 F0 = 0.04;
    //F0 = 0.2 * spec;
    F0 = lerp(F0, albedo, metallic);
    
    // reflectance equation
    float3 Lo = 0;
    {
        // ライトの輝度を求める
        float3 L = normalize(-LightDir); // 光方向ベクトル
        float3 H = normalize(V + L); // マイクロファセット法線
        float attenuation = 1.0; // 距離減衰
        float3 radiance = LightColor * attenuation; // 輝度
        
        // cook-torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        float3 kS = F; // 鏡面反射率
        float3 kD = 1.0 - kS; // 拡散反射率
        kD *= 1.0 - metallic;
        
        // specularを求める
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        float3 specular = numerator / denominator;
        
        // 最終的な放射輝度
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    
    // シャドウマップ
    float shadow = 1;
    float4 possm = GetShadowMapPosition(worldPos);
    if (possm.x > 0 && possm.x < 1 && possm.y > 0 && possm.y < 1)
    {
        shadow = VarianceShadow(possm);
    }
    float3 shadowColor = inverseLerp(0, 1, shadow);
    //shadowColor += inverseLerp(0, 0.5, shadow) * float3(0.2, 0.05, 0.05);
    shadowColor = saturate(shadowColor);
    
    // Image-Based Lighting
    float3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    float maxLod = querySpecularTextureLevels() - 1;
    
    //float3 irradiance = gDiffuseMap.Sample(gSampler, N).rgb;
    float3 irradiance = gSpecularMap.SampleLevel(gSampler, N, maxLod).rgb;
    float3 diffuse = irradiance * albedo;
    //diffuse = albedo;
    //float3 diffuse = lerp(0.8, 1, irradiance) * albedo;
    
    float3 prefilteredColor = gSpecularMap.SampleLevel(gSampler, R, roughness * maxLod).rgb;
    float3 envBRDF = gBrdfLUT.Sample(gSampler, float2(clamp(dot(N, V), 0.0, 0.99), 1.0 - roughness)).rgb;
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    
    float3 ambient = (kD * diffuse + specular) * ao;
    float3 color = ambient + Lo * shadowColor;
    
    // フォグ
    float fog = inverseLerp(100, 300, depth);
    float3 fogColor = float3(1, 1, 1);
    color = lerp(color, fogColor, fog);
      
    return float4(color, 1.0);
}