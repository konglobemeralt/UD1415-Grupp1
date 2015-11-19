struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float3 Normal : NORMAL;
	float2 uv : TEXCOORD;
};

float4 main(VS_OUT input) : SV_TARGET
{
	float4 textureColor = float4(0.5, 0.5, 0.5, 1.0);
	return textureColor;
}