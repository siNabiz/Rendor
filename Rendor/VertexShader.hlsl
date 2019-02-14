struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

// Per-pixel color data passed through the pixel shader.
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

VS_OUTPUT main(VS_INPUT IN) // main is the default function name
{
    VS_OUTPUT Output;

    Output.Position = float4(IN.Position, 1.0f);
    Output.Color = float4(IN.Color, 1.0f);

    return Output;
}