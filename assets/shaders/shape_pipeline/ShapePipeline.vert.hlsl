struct VSIn
{
    float3 position : POSITION0;
    float3 normal : NORMAL;

    float4 modelRow0;
    float4 modelRow1;
    float4 modelRow2;
    float4 modelRow3;

    float4 color;

    float specularStrength;
    int shininess;
};

struct VSOut
{
    float4 position : SV_POSITION;

    float3 worldPosition : POSITION0;
    float3 worldNormal : NORMAL;

    float4 color : COLOR0;
    float specularStrength;
    int shininess;
};

struct CameraBuffer {
    float4x4 viewProjection;
    float3 position;
    float placeholder;
};
ConstantBuffer <CameraBuffer> camera: register(b0, space0);

VSOut main(VSIn input) {
    VSOut output;

    float4x4 model = float4x4(input.modelRow0, input.modelRow1, input.modelRow2, input.modelRow3);

    // This multiply here is delibrate
    float4 worldPosition = model * float4(input.position, 1.0);
    output.worldPosition = worldPosition.xyz;
    output.worldNormal = (model * float4(input.normal, 0.0f)).xyz;

    output.position = mul(camera.viewProjection, worldPosition);

    output.color = input.color;
    output.specularStrength = input.specularStrength;
    output.shininess = input.shininess;

    return output;
}