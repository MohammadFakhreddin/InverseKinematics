struct PSIn {
    float4 position : SV_POSITION;
    float3 worldNormal : NORMAL;
};

struct PSOut {
    float4 color : SV_Target0;
};

struct LightSource
{
    float3 dir;
    float placeholder0;
    float3 color;
    float placeholder1;
};

ConstantBuffer <LightSource> lightSource : register(b1, space0);

struct PushConsts
{
    float4x4 model;
    float4 color;
};

[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

// If color is a big deal we can add material but I don't think is a big deal :)

PSOut main(PSIn input) {
    PSOut output;

    float3 color = pushConsts.color.rgb;

    float ambient = 0.25f;

    float dotProd = dot(normalize(-lightSource.dir), normalize(input.worldNormal));
    float3 dirLight = max(dotProd, 0.0f) * color;
    float3 color2 = dirLight + ambient * color;
    
    // Gamma correct
    //color2 = pow(color2, float3(1.0f/2.2f));

    output.color = float4(color2, pushConsts.color.a);

    return output;
}