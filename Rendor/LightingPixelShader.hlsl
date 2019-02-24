Texture2D Texture0 : register(t0);
Texture2D Texture1 : register(t1);
sampler Sampler : register(s0);

cbuffer AppData : register(b0)
{
    float3 CameraPosition;
    float Padding;
};

struct _Light
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float3 LightPosition;
    float Padding;
};

cbuffer Light : register(b1)
{
    _Light Light;
};

struct _Material
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float Shininess;
    bool UseTexture;
    float2 Padding;
};

cbuffer Material : register(b2)
{
    _Material Material;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float3 FragPosition : TEXCOORD1;
    float3 FragNormal : TEXCOORD2;
};

float4 main(PS_INPUT IN) : SV_TARGET
{
    // calculate various directions
    float3 lightDirection = normalize(Light.LightPosition - IN.FragPosition);
    float3 normalDirection = normalize(IN.FragNormal);
    float3 reflectionDirection = normalize(reflect(-lightDirection, normalDirection));
    float3 viewDirection = normalize(CameraPosition - IN.FragPosition);

    // Ambient light
    float4 ambient = Material.Ambient * Light.Ambient;

    // Diffuse light
    float diffuseValue = max(dot(lightDirection, normalDirection), 0);
    float4 diffuse = diffuseValue * Material.Diffuse * Light.Diffuse;

    // Specular light
    float specularValue = pow(max(dot(viewDirection, reflectionDirection), 0), Material.Shininess);
    float4 specular = specularValue * Material.Specular * Light.Specular;

    // Apply textures
    float4 texColor = 1;
    if (Material.UseTexture)
    {
        float4 texColor0 = Texture0.Sample(Sampler, IN.TexCoord);
        float4 texColor1 = Texture1.Sample(Sampler, IN.TexCoord);

        texColor = lerp(texColor0, texColor1, 0.2);
    }

    float4 finalColor = (ambient + diffuse + specular) * texColor;

    return finalColor;
}