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
Texture2D gDepth : register(t3);
Texture2D gShadowMap : register(t4);

SamplerState gSampler : register(s0); // サンプラー


float inverseLerp(float a, float b, float value)
{
    return saturate((value - a) / (b - a));
}

float hash(float2 input)
{
    return frac(1.0e4 * sin(17.0 * input.x + 0.1 * input.y) * (0.1 + abs(sin(13.0 * input.y + input.x))));
}

float hash3D(float3 input)
{
    return hash(float2(hash(input.xy), input.z));
}

float hashedAlpha(float4 objCoord)
{
    float g_HashScale = 1.0;

    float maxDeriv = max(length(ddx(objCoord.xyz)), length(ddy(objCoord.xyz)));
    float pixScale = 1.0 / (g_HashScale * maxDeriv);
    
    float2 pixScales = float2(exp2(floor(log2(pixScale))), exp2(ceil(log2(pixScale))));
    
    float2 alpha = float2(hash3D(floor(pixScales.x * objCoord.xyz)), hash3D(floor(pixScales.y * objCoord.xyz)));

    float lerpFactor = frac(log2(pixScale));
    
    float x = (1 - lerpFactor) * alpha.x + lerpFactor * alpha.y;
    
    float a = min(lerpFactor, 1 - lerpFactor);
    float3 cases = float3(x * x / (2 * a * (1 - a)), (x - 0.5 * a) / (1 - a), 1.0 - ((1 - x) * (1 - x) / (2 * a * (1 - a))));

    float res = (x < (1 - a)) ? ((x < a) ? cases.x : cases.y) : cases.z;

    return clamp(res, 1.0e-6, 1.0);
}

// Phong鏡面反射光を計算する
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 toEye, float3 normal, float shinePower)
{
    // 反射ベクトルを求める
    float3 refVec = normalize(reflect(lightDirection, normal));
    // 光が当たったサーフェイスから視点に伸びるベクトルを求める
    toEye = normalize(toEye);
    // 鏡面反射の強さを求める
    float t = dot(refVec, toEye);
    // 鏡面反射の強さを0以上の数値にする
    t = max(0.0f, t);
    // 鏡面反射の強さを絞る
    t = pow(t, shinePower);
    // 鏡面反射光を求める
    return lightColor * t;
}





float rand(float2 pos, int seed)
{
    float dot_product = dot(seed, pos);
    return frac(sin(dot_product) * 43758.5453);
}

float PoissonShadow(float4 possm, int n, float size)
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
    
    float sum = 0;
    float distance = possm.z / possm.w;
    for (int i = 0; i < n; i++)
    {
        int index = int(16.0 * rand(possm.xy, i)) % 16;
        float2 offset = poissonDisk[index] * size / 1024;
        float sm = gShadowMap.Sample(gSampler, saturate(possm.xy + offset)).r;
        if (distance < sm)
        {
            sum += 1;
        }
    }
    return sum / n;
}

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


float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    float4 worldPos = gPosition.Sample(gSampler, uv);
    
    if (worldPos.w < 0.5)
    {
        discard;
    }
    
    float4 texNormal = gNormal.Sample(gSampler, uv);
    float4 texAlbedo = gAlbedo.Sample(gSampler, uv);
    float4 texDepth = gDepth.Sample(gSampler, uv);
    
    float3 normal = normalize(texNormal.xyz);
    float3 albedo = texAlbedo.xyz;
    float shininess = texNormal.w;
    float depth = texDepth.r;
    float outlineMask = texAlbedo.w;
    
    
    float3 viewDir = normalize(worldPos.xyz - CameraPos);
    float3 shadowColor = saturate(0.7 * normalize(albedo.rgb)) + 0.3;
    //float3 shadowColor = saturate(0.8 * normalize(diffuse.rgb)) + 0.1;
    
    // リム陰
    float rimPower = 1 - dot(normal, -viewDir);
    float rimShade = lerp(0.5, 1, inverseLerp(0.98, 1, 1 - pow(rimPower, 3)));
    rimShade *= inverseLerp(0.98, 1, 1 - pow(rimPower, 5));
    float3 rimShadeColor = lerp(saturate(0.5 * normalize(albedo.rgb)) + 0.5, 1, rimShade);
    
    // リムマスク
    float rimMask = pow(rimPower, 8);
    rimMask = inverseLerp(0, 0.2, rimMask);
    rimShadeColor = saturate(rimShadeColor + rimMask);
    
    // リムライト
    float3 rimColor = 0.2; //normalize(diffuse.rgb) * 2.5;
    float rimLightPower = max(0, dot(normal, -LightDir));
    float rimLight = pow(saturate(rimPower * rimLightPower), 5);
    rimLight = inverseLerp(0, 0.2, rimLight);
    float3 rimLightColor = rimColor * rimLight;
    
    // 陰
    float shade = dot(normal, -LightDir);
    shade = max(0, shade);
    shade = pow(shade * 0.5 + 0.5, 2);
    //shade = shade * 0.5f + 0.5f;
    //p = (float) round(p);
    shade = inverseLerp(0.25, 0.4, shade);
    
    // スペキュラ
    float3 specularLightColor = CalcPhongSpecular(-LightDir, 0.2, viewDir, normal, shininess);
    
    // シャドウマップ
    float4 possm = GetShadowMapPosition(worldPos);
    if (possm.x >= 0 && possm.x < 1 && possm.y >= 0 && possm.y < 1)
    {
        //float z = possm.z / possm.w;
        //float4 sm = gShadowMap.Sample(gSampler, possm.xy);
        //shade = (z - 0.002 < sm.x) ? shade : 0;
        
        shade = min(PoissonShadow(possm, 10, 1.5), shade);
        
    }
    
    float3 shadeColor = lerp(shadowColor, 1, shade);
    rimLightColor *= lerp(0.5, 1.0, shade);
    specularLightColor *= lerp(0, 1.0, shade);
    
    // フォグ
    float fog = inverseLerp(0.4, 0.5, inverseLerp(0, 120, depth));
    float3 fogColor = float3(0.7, 0.7, 0.7);
    
    float4 color = float4(albedo * shadeColor * rimShadeColor + rimLightColor + specularLightColor, 1);
    //color.rgb = lerp(color.rgb, fogColor, fog);
    
    return color;
    //return float4(outlineMask, 0, 0, 1);
    //return float4(input.viewPos.x, input.viewPos.y, -input.viewPos.z, 1);
}