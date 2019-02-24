Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);
sampler Sampler : register(s0);

cbuffer AppData : register(b0)
{
    float4 LightColor;
    float4 ObjectColor;
    float3 LightPosition;
    bool UseConstantColor;
    float3 CameraPosition;
    bool UseTexture;
};


struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 FragPosition : TEXCOORD1;
    float3 FragNormal : TEXCOORD2;
};

float4 main(PS_INPUT IN) : SV_TARGET
{
    float4 objectColor = 0;
    if (UseConstantColor)
    {
        objectColor = ObjectColor;
    }
    else
    {
        objectColor = IN.Color;
    }

    // Apply textures
    if (UseTexture)
    {
        float4 texColor0 = Texture0.Sample(Sampler, IN.TexCoord);
        float4 texColor1 = Texture1.Sample(Sampler, IN.TexCoord);

        objectColor *= lerp(texColor0, texColor1, 0.2);
    }

    // Ambient light
    float ambientStrength = 0.1;
    float4 ambient = ambientStrength * LightColor;

    float3 lightDirection = normalize(LightPosition - IN.FragPosition);
    float3 normalDirection = normalize(IN.FragNormal);

    // Diffuse light
    float diffuseValue = max(dot(lightDirection, normalDirection), 0);
    float diffuse = diffuseValue * LightColor;

    // Specular light
    float specularStrength = 0.5;
    float specularPower = 128;
    float3 reflectionDirection = normalize(reflect(-lightDirection, normalDirection));
    float3 viewDirection = normalize(CameraPosition - IN.FragPosition);

    float specularValue = pow(max(dot(viewDirection, reflectionDirection), 0), specularPower);
    float4 specular = specularValue * specularStrength * LightColor;

    // Full lighting
    float4 lightingResult = float4((float3)(objectColor * (ambient + diffuse + specular)), 1.0f);

    return lightingResult;
}