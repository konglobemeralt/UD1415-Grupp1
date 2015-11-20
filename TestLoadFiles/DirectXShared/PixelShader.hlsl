struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float3 Normal : NORMAL;
	float2 uv : TEXCOORD;
};

Texture2D txDiffuse : register(t0);
SamplerState stSampler : register(s0);

float4 main(VS_OUT input) : SV_TARGET
{
	float4 textureColor = float4(0.5, 0.5, 0.5, 1.0);
	textureColor = txDiffuse.Sample(stSampler, input.uv);
	return textureColor;
}