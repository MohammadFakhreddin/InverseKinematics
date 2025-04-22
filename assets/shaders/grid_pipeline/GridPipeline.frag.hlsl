struct PSIn
{
    float4 position : SV_POSITION;
};

struct PSOut
{
    float4 color : SV_Target0;
};

struct PushConsts
{
    float3 color;
    float thickness;
};
[[vk::push_constant]]
cbuffer {
    PushConsts pushConsts;
};

PSOut main(PSIn input) {
    PSOut output;

    output.color = float4(1.0f, 1.0f, 0.0f, 1.0f);

    return output;
}