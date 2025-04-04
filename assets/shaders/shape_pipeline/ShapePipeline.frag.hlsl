struct PSIn
{
    float4 position : SV_POSITION;

    float3 worldPosition : POSITION0;
    float3 worldNormal : NORMAL;

    float4 color : COLOR0;

    float specularStrength;
    int shininess;
};

struct PSOut
{
    float4 color : SV_Target0;
};

struct CameraBuffer {
    float4x4 viewProjection;
    float3 position;
    float placeholder;
};
ConstantBuffer <CameraBuffer> camera: register(b0, space0);

struct LightSource
{
    float3 direction;
    float ambientStrength;
    float3 color;
    float placeholder1;
};
// TODO: Use https://github.com/KhronosGroup/SPIRV-Reflect to make your life easier
ConstantBuffer <LightSource> light : register(b1, space0);

PSOut main(PSIn input) {
    PSOut output;
    output.color = float4(1.0, 0.0, 0.0, 1.0);
    return output;

    float ambient = light.ambientStrength * light.color;

    float3 fragNormal = normalize(input.worldNormal);

    float3 lightDir = -normalize(light.direction);
    float3 lightColor = light.color;

    float diffuseDot = dot(lightDir, fragNormal);

    float3 diffuse = max(diffuseDot, 0.0f) * light.color;

    float3 viewDir = normalize(camera.position - input.worldPosition);
    float3 reflectDir = reflect(lightDir, fragNormal);

    float3 specular = float3(0.0, 0.0, 0.0);
    if (diffuseDot > 0.0)
    {
        float3 specularStrength2 = float3(1) * input.specularStrength;
        specular = specularStrength2 * pow(max(dot(viewDir, reflectDir), 0.0), input.shininess) * lightColor;
    }

    output.color = float4((ambient + diffuse + specular), 1.0) * input.color;

    return output;
}