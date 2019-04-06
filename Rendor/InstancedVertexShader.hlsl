cbuffer AppData : register(b0)
{
    matrix ViewProjectionMatrix;
    matrix LightSpaceViewProjectionMatrix;
};

struct VS_INPUT
{
    // per-vertex data
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD;
    // per-instance data
    matrix ModelMatrix : MODELMATRIX;
    matrix NormalMatrix : NORMALMATRIX;
};

// Per-pixel color data passed through the pixel shader.
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 FragPosition : TEXCOORD1;
    float3 FragNormal : TEXCOORD2;
    float4 FragPositionLightSpace : TEXCOORD3;
};

VS_OUTPUT main(VS_INPUT IN) // main is the default function name
{
    VS_OUTPUT Output;
    
    matrix MVP = mul(ViewProjectionMatrix, IN.ModelMatrix);
    matrix light_MVP = mul(LightSpaceViewProjectionMatrix, IN.ModelMatrix);

    Output.Position = mul(MVP, float4(IN.Position, 1.0f));
    Output.TexCoord = IN.TexCoord;

    Output.FragPosition = (float3)mul(IN.ModelMatrix, float4(IN.Position, 1.0f));
    Output.FragNormal = mul((float3x3) IN.NormalMatrix, IN.Normal);

    Output.FragPositionLightSpace = mul(light_MVP, float4(IN.Position, 1.0f));

    return Output;
}