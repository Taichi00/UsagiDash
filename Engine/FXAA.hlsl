struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

Texture2D gTex : register(t0);

SamplerState gSampler : register(s0); // �T���v���[


float FxaaLuma(float4 rgba)
{
//(ax * bx + ay * by + az * bz) NTSC�O���[�X�P�[����
    return dot(rgba.rgb, float3(0.299, 0.587, 0.114));
}

float4 FXAA(Texture2D tex, float2 uv)
{
    //���͂̍��W�擾�p
    float2 screenSize;
    tex.GetDimensions(screenSize.x, screenSize.y);
    float2 Pos_h = (0.5f / screenSize.x, 0.5f / screenSize.y);
    float2 Pos_t = (2.0f / screenSize.x, 2.0f / screenSize.y);

    //���͂̍��W
    float4 ConsolePos = float4(uv.x - Pos_h.x, uv.y - Pos_h.y, uv.x + Pos_h.x, uv.y + Pos_h.y);

    float luma_LT = FxaaLuma(tex.Sample(gSampler, ConsolePos.xy));
    float luma_LB = FxaaLuma(tex.Sample(gSampler, ConsolePos.xw));
    float luma_RT = FxaaLuma(tex.Sample(gSampler, ConsolePos.zy)) + 0.002604167;
    float luma_RB = FxaaLuma(tex.Sample(gSampler, ConsolePos.zw));
    float4 luma_C = tex.Sample(gSampler, uv); //����

    //���͂̍ő�ŏ�
    float luma_Max = max(max(luma_RT, luma_RB), max(luma_LT, luma_LB));
    float luma_Min = min(min(luma_RT, luma_RB), min(luma_LT, luma_LB));

    //�ő�Ɠx��1/8
    float lumaMaxScaledClamped = max(0.05, luma_Max * 0.125);

    //�O���[�X�P�[��
    float luma_M = FxaaLuma(luma_C);
    //�Ɠx��
    float lumaMaxSubMinM = max(luma_Max, luma_M) - min(luma_Min, luma_M);

    //�ω����r
    if (lumaMaxSubMinM < lumaMaxScaledClamped)
    {
        //�ω������Ȃ��ꍇ�͌��̐F��Ԃ�
        return luma_C;
    }
    else
    {
        //�e�����̏Ɠx��
        float dirSwMinusNe = luma_LB - luma_RT;
        float dirSeMinusNw = luma_RB - luma_LT;

        //�Ɠx�x�N�g��
        float2 dir1 = normalize(float2(dirSwMinusNe + dirSeMinusNw, dirSwMinusNe - dirSeMinusNw));
        //�Ɠx�x�N�g���̍�
        float2 dirAbsMinTimesC = dir1 / (8.0 * min(abs(dir1.x), abs(dir1.y)));
        //�͈͂Ɏ��߂�
        float2 dir2 = clamp(dirAbsMinTimesC, -2.0, 2.0) * Pos_t;

        //�ړ��ʎZ�o
        dir1 *= Pos_h;
        //�e�����擾
        float4 rgbyN1 = tex.Sample(gSampler, uv - dir1); //���h�b�g�~�Ɠx�x�N�g�� ����
        float4 rgbyP1 = tex.Sample(gSampler, uv + dir1); //���h�b�g�~�Ɠx�x�N�g�� �E��
        float4 rgbyN2 = tex.Sample(gSampler, uv - dir2); //�Q�h�b�g�~�Ɠx�x�N�g�� ����
        float4 rgbyP2 = tex.Sample(gSampler, uv + dir2); //�Q�h�b�g�~�Ɠx�x�N�g�� �E��
        //���Z�����l
        float4 rgbyA = rgbyN1 + rgbyP1; //���Z���Č��ʂ�ۑ�
        //1/4�����l
        float4 rgbyB = (rgbyN2 + rgbyP2 + rgbyA) * 0.25;

        //�O���[�X�P�[��
        float rgbyBM = FxaaLuma(rgbyB);
        //���l����
        if ((rgbyBM < luma_Min) || (rgbyBM > luma_Max))
        {
            //1/2��Ԃ�
            return rgbyA * 0.5;
        }
        else
        {
            //1/4��Ԃ�
            return rgbyB;
        }
    }
}

//#define FXAA_PC 1
//#define FXAA_HLSL_5 1
////#define FXAA_WUALITY__PRESET 12
//#include "FXAA3.cginc"

//#define PIXEL_SIZE float2(1.0 / 1280.0, 1.0 / 720.0)
//#define SUBPIX 0.00001
//#define EDGE_THRESHOLD 0.1
//#define EDGE_THRESHOLD_MIN 0

float4 main(VSOutput input) : SV_TARGET
{
    float2 uv = input.UV;
    
    //float4 color = FXAA(gTex, uv);
    float4 color = gTex.Sample(gSampler, uv);

    //uint dx, dy;
    //gTex.GetDimensions(dx, dy);
    //float2 rcpro = rcp(float2(dx, dy));
    
    //FxaaTex InputFXAATex;
    //InputFXAATex.smpl = gSampler;
    //InputFXAATex.tex = gTex;
    
    //float4 color = FxaaPixelShader(
    //    uv, // FxaaFloat2 pos,
    //    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsolePosPos,
    //    InputFXAATex, // FxaaTex tex,
    //    InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegOne,
    //    InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegTwo,
    //    rcpro, // FxaaFloat2 fxaaQualityRcpFrame,
    //    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt,
    //    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
    //    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
    //    0.75, // FxaaFloat fxaaQualitySubpix,
    //    0.125, // FxaaFloat fxaaQualityEdgeThreshold,
    //    0.0312, // FxaaFloat fxaaQualityEdgeThresholdMin,
    //    0.0f, // FxaaFloat fxaaConsoleEdgeSharpness,
    //    0.0f, // FxaaFloat fxaaConsoleEdgeThreshold,
    //    0.0f, // FxaaFloat fxaaConsoleEdgeThresholdMin,
    //    FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f) // FxaaFloat fxaaConsole360ConstDir,
    //);
    
    return color;
}