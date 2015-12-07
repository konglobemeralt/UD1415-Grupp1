struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D txDiffuse : register(t0);
SamplerState stSampler : register(s0);

float4 main(VS_OUT input) : SV_TARGET
{
	float4 textureColor = float4(0.5f, 0.5f, 0.5f, 1.0f);
	textureColor = txDiffuse.Sample(stSampler, input.uv);
	return textureColor;
}