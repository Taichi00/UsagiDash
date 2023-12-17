struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

#define PI 3.14159265359
#define E 2.71828182846

static int SAMPLES = 8;
static float BlurSize = 0.001;
static float StandardDeviation = 0.5;

Texture2D gTex : register(t0);

SamplerState gSampler : register(s0); // ƒTƒ“ƒvƒ‰[

float4 main(VSOutput input) : SV_TARGET
{
    float4 color = 0;
    
    float sum = 0;
    
    for (float i = 0; i < SAMPLES; i++)
    {
        float offset = (i / (SAMPLES - 1) - 0.5) * BlurSize;
        float2 uv = input.UV + float2(offset, 0);

        float stDevSquared = StandardDeviation * StandardDeviation;
        float gauss = (1 / sqrt(2 * PI * stDevSquared)) * pow(E, -((offset * offset) / (2 * stDevSquared)));

        sum += gauss;
        color += gTex.Sample(gSampler, uv) * gauss;
    }

    color = color / sum;
    //color = StandardDeviation;
    
    return color;
}