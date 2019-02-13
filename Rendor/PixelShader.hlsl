cbuffer ColorBuffer : register(b0)
{
    float4 Color;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
};

float4 main(PS_INPUT IN) : SV_TARGET
{
    return Color;
}