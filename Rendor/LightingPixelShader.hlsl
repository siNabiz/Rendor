#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1   
#define SPOT_LIGHT 2

#define NUMBER_OF_LIGHTS 1

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
    float3 Position;
    float Intensity;
    float3 Direction;
    float SpotAngle;
    float ConstantAttenuation;
    float LinearAttenuation;
    float QuadraticAttenuation;
    int Type;
};

cbuffer Light : register(b1)
{
    _Light Lights[NUMBER_OF_LIGHTS];
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

struct LightingResult
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
};

void DoGeneralLighting(inout LightingResult lightingResult,
                       _Light light,
                       float4 materialAmbient, float4 materialDiffuse, float4 materialSpecular,
                       float3 lightDirection, float3 normalDirection, float3 viewDirection)
{
    // Ambient light
    lightingResult.Ambient = materialAmbient * light.Ambient * light.Intensity;

    // Diffuse light
    float diffuseValue = max(dot(lightDirection, normalDirection), 0);
    lightingResult.Diffuse = diffuseValue * materialDiffuse * light.Diffuse * light.Intensity;

    // Specular light
    //float3 reflectionDirection = normalize(reflect(-lightDirection, normalDirection));
    //float specularValue = pow(max(dot(viewDirection, reflectionDirection), 0), Material.Shininess);
    
    float3 halfwayDirection = normalize(-lightDirection + viewDirection);
    float specularValue = pow(max(dot(normalDirection, halfwayDirection), 0), Material.Shininess);

    lightingResult.Specular = specularValue * materialSpecular * light.Specular * light.Intensity;
}

LightingResult DoDirectionalLight(_Light light,
                                  float4 materialAmbient, float4 materialDiffuse, float4 materialSpecular,
                                  float3 normalDirection, float3 viewDirection)
{
    LightingResult output;

    DoGeneralLighting(output, light, materialAmbient, materialDiffuse, materialSpecular, light.Direction, normalDirection, viewDirection);

    return output;
}

void DoAttenuation(inout LightingResult lightingResult,
                   float lightDistance,
                   float constantAttenuation, float linearAttenuation, float quadraticAttenuation)
{
    float attenuation = 1.0f / (constantAttenuation +
                                (linearAttenuation * lightDistance) +
                                (quadraticAttenuation * lightDistance * lightDistance));
    
    lightingResult.Ambient *= attenuation;
    lightingResult.Diffuse *= attenuation;
    lightingResult.Specular *= attenuation;
}

LightingResult DoPointLight(_Light light,
                            float4 materialAmbient, float4 materialDiffuse, float4 materialSpecular,
                            float3 lightDirection, float3 normalDirection, float3 viewDirection,
                            float lightDistance)
{
    LightingResult output;

    DoGeneralLighting(output, light, materialAmbient, materialDiffuse, materialSpecular, lightDirection, normalDirection, viewDirection);
    DoAttenuation(output, lightDistance, light.ConstantAttenuation, light.LinearAttenuation, light.QuadraticAttenuation);

    return output;
}

void DoSpotAngle(inout LightingResult lightingResult,
                 float3 lightDirection, float3 lightSpotDirection, float lightSpotAngle)
{
    float minCos = cos(lightSpotAngle);
    float maxCos = (1.0f + minCos) * 0.5f;
    float cosValue = dot(lightSpotDirection, -lightDirection);

    float attenuation = smoothstep(minCos, maxCos, cosValue);

    lightingResult.Ambient *= attenuation;
    lightingResult.Diffuse *= attenuation;
    lightingResult.Specular *= attenuation;
}

LightingResult DoSpotLight(_Light light,
                           float4 materialAmbient, float4 materialDiffuse, float4 materialSpecular,
                           float3 lightDirection, float3 normalDirection, float3 viewDirection,
                           float lightDistance)
{
    LightingResult output;

    DoGeneralLighting(output, light, materialAmbient, materialDiffuse, materialSpecular, lightDirection, normalDirection, viewDirection);
    DoAttenuation(output, lightDistance, light.ConstantAttenuation, light.LinearAttenuation, light.QuadraticAttenuation);
    DoSpotAngle(output, lightDirection, light.Direction, light.SpotAngle);

    return output;
}

float4 main(PS_INPUT IN) : SV_TARGET
{
    // calculate various directions
    float3 normalDirection = normalize(IN.FragNormal);
    float3 viewDirection = normalize(CameraPosition - IN.FragPosition);

    // Use material lighting textures (maps)
    float4 materialAmbient = Material.Ambient;
    float4 materialDiffuse = Material.Diffuse;
    float4 materialSpecular = Material.Specular;
    if (Material.UseTexture)
    {
        float4 diffuseMap = Texture0.Sample(Sampler, IN.TexCoord);
        float4 specularMap = Texture1.Sample(Sampler, IN.TexCoord);

        materialAmbient = diffuseMap;
        materialDiffuse = diffuseMap;
        materialSpecular = specularMap;
    }
    
    LightingResult result;
    result.Ambient = 0;
    result.Diffuse = 0;
    result.Specular = 0;

    [unroll]
    for (int i = 0; i < NUMBER_OF_LIGHTS; i++)
    {
        _Light light = Lights[i];
        float lightDistance = length(light.Position - IN.FragPosition);
        float3 lightDirection = normalize(light.Position - IN.FragPosition);
        LightingResult output;
        switch (light.Type)
        {
            case (DIRECTIONAL_LIGHT):
                output = DoDirectionalLight(light, 
                                            materialAmbient, materialDiffuse, materialSpecular,
                                            normalDirection, viewDirection);
                break;
            case (POINT_LIGHT):
                output = DoPointLight(light, 
                                      materialAmbient, materialDiffuse, materialSpecular,
                                      lightDirection, normalDirection, viewDirection,
                                      lightDistance);
                break;
            case (SPOT_LIGHT):
                output = DoSpotLight(light, materialAmbient, materialDiffuse, materialSpecular,
                                     lightDirection, normalDirection, viewDirection, lightDistance);
                break;
            default:
                break;
        }

        result.Ambient += output.Ambient;
        result.Diffuse += output.Diffuse;
        result.Specular += output.Specular;
    }

    float4 finalColor = result.Ambient + result.Diffuse + result.Specular;

    return finalColor;
}