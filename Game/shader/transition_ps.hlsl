
cbuffer constants : register(b0)
{
    float4 transition_color : packoffset(c0);
    float time : packoffset(c1.x);
    float aspect : packoffset(c1.y);
    float2 direction : packoffset(c1.z);
    float inverse : packoffset(c2.x);
}

Texture2D InputTexture : register(t0);
SamplerState InputSampler : register(s0);

// �~�̋����֐�
float Circle(float2 p)
{
    return dot(p, p);
}

float4 main(
    float4 position : SV_POSITION,
    float4 scene_position : SCENE_POSITION,
    float4 uv0 : TEXCOORD0
    ) : SV_Target
{
    float2 uv = uv0.xy;
    float4 color;
    
    float division_size = 30; // ������
    float width = 0.1; // �t�F�[�h�̕�
    float2 dir = normalize(direction); // �t�F�[�h�̕���
    
    float2 div = float2(division_size, division_size * aspect);
    float val = time * (dot(div - 1.0, abs(dir)) * width + 2.0);
    
    float2 st = uv * div;
    float2 i_st = floor(st);
    
    // uv���W�� -1 ~ 1 �ɕϊ�
    float2 pos = frac(st) * 2 - 1;
    
    float a = 1;
    float2 sg = sign(dir);
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            float2 f = (div - 1.0) * (0.5 - sg * 0.5) + (i_st + float2(i, j)) * sg;
            float v = val - dot(f, abs(dir)) * width;

            float ci = Circle(pos - float2(2.0 * i, 2.0 * j));
            
            a = min(a, step(v, ci));
        }
    }
    
    a = inverse - a * (inverse * 2 - 1);
    
    color.rgb = transition_color.rgb * a;
    color.a = a;
    
    return color;
}