struct VS_OUT
{
	float4 Pos : SV_POSITION;
};

float4 main(VS_OUT input) : SV_TARGET
{
	float4 textureColor = float4(1.0, 1.0, 1.0, 1.0);
	return textureColor;
}