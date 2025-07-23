#include "Test.hlsli"

Texture2D<float32_t4> gTexture : register(t0); //SRV register=>t
SamplerState gSampler : register(s0); //Sampler register=>s

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input){
    PixelShaderOutput output;
    float32_t2 uv = input.texcoord;
    float32_t4 textureColor = gTexture.Sample(gSampler, uv);
    
    // grayscale
    float32_t value = dot(textureColor.rgb, float32_t3(0.2125f, 0.154f, 0.0721f));
    output.color = float32_t4(value, value, value, textureColor.a);
    output.color.rgb = value * float32_t3(1.0f, 74.0f / 107.0f, 43.0f / 107.0f);
    return output;
}