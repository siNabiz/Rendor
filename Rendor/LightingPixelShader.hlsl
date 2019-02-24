Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);
sampler Sampler : register(s0);

cbuffer AppData : register(b0)
{
    float4 LightColor;
    float4 ObjectColor;
    bool UseConstantColor;
    bool UseTexture;
};


struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

float4 main(PS_INPUT IN) : SV_TARGET
{
    float4 colorVal = 0;
    if (UseConstantColor)
    {
        colorVal = ObjectColor;
    }
    else
    {
        colorVal = IN.Color;
    }

    colorVal *= LightColor;

    if (UseTexture)
    {
        float4 texColor0 = Texture0.Sample(Sampler, IN.TexCoord);
        float4 texColor1 = Texture1.Sample(Sampler, IN.TexCoord);

        return (colorVal * lerp(texColor0, texColor1, 0.2));
    }
    else
    {
        return colorVal;
    }
}