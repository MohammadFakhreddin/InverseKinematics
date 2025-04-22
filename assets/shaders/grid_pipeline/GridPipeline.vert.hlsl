struct VSIn
{
    float3 position : POSITION0;
};

struct VSOut
{
    float4 position : SV_POSITION;
};

VSOut main(VSIn input) {
    VSOut output;

    output.position = float4(input.position.x, input.position.y, 1.0 - 1e-5, 1.0);

    return output;
}