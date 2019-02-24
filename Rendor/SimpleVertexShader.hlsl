cbuffer AppData : register(b0)
{
    matrix MVPMatrix;
};

struct VS_INPUT
{
    // per-vertex data
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
};

// Per-pixel color data passed through the pixel shader.
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT IN) // main is the default function name
{
    VS_OUTPUT Output;
    
    Output.Position = mul(MVPMatrix, float4(IN.Position, 1.0f));
    Output.TexCoord = IN.TexCoord;

    return Output;
}