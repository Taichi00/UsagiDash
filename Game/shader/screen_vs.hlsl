struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

VSOutput main(uint vertexId : SV_VertexId)
{
    float x = (vertexId >> 1) * 2.0 - 1.0;
    float y = 1.0 - (vertexId & 1) * 2.0;
    
    VSOutput result = (VSOutput) 0;
    result.Position = float4(x, y, 0, 1);
    
    float u = (vertexId >> 1);
    float v = (vertexId & 1);
    
    result.UV = float2(u, v);
    
    return result;
}