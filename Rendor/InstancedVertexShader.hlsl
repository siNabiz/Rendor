cbuffer AppData : register(b0)
{
    matrix ViewProjectionMatrix;
};

struct VS_INPUT
{
    // per-vertex data
    float3 Position : POSITION;
    float3 Color : COLOR;
    float2 TexCoord : TEXCOORD;
    // per-instance data
    matrix ModelMatrix : MODELMATRIX;
};

// Per-pixel color data passed through the pixel shader.
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT IN) // main is the default function name
{
    VS_OUTPUT Output;
    
    matrix MVP = mul(ViewProjectionMatrix, IN.ModelMatrix);
    
    Output.Position = mul(MVP, float4(IN.Position, 1.0f));
    Output.Color = float4(IN.Color, 1.0f);
    Output.TexCoord = IN.TexCoord;

    return Output;
}