struct PSIn
{
    float3 windowPosition : POSITION0;
};

struct PSOut
{
    float4 color : SV_Target0;
};
// TODO: Pass density and camera transform here
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

    float3 position = (input.windowPosition) * 0.5 + 0.5;
    position *= 10.0f;// TODO: Expose a value for density
    float xMod = fmod(position.x, 1.0);
    float yMod = fmod(position.y, 1.0);
    if (
        (xMod < pushConsts.thickness || xMod > 1.0 - pushConsts.thickness) ||
        (yMod < pushConsts.thickness || yMod > 1.0 - pushConsts.thickness)
    )
    {
        output.color = float4(pushConsts.color, 1.0);
    }
    else
    {
        discard;
    }

    return output;
}