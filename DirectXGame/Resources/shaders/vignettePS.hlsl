#include "Vignette.hlsli"

Texture2D<float32_t4> gTexture : register(t0); //SRV register=>t
SamplerState gSampler : register(s0); //Sampler register=>s

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input){
    PixelShaderOutput output;
    float32_t2 uv = input.texcoord;
    float32_t4 textureColor = gTexture.Sample(gSampler, uv);
    
    // 周囲を0に、中心になるほど明るくなるように計算で調整
    float32_t2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    // correctだけっで計算すると中心の最大値が0.0625で暗すぎるのでSceleで調整。この例では16倍にして1にしている
    float vignette = correct.x * correct.y * 16.0f;
    //とりあえず0.8乗でそれっぽくしてみた
    vignette = saturate(pow(vignette, 0.8f));
    // grayscale
    float32_t value = dot(textureColor.rgb, float32_t3(0.2125f, 0.154f, 0.0721f));
    output.color = float32_t4(value, value, value, textureColor.a);
    //output.color.rgb *= vignette * float32_t3(value, value, value);
    // セピア色
    output.color.rgb *= vignette * (value * float32_t3(1.0f, 74.0f / 107.0f, 43.0f / 107.0f));
    return output;
}