cbuffer worldMatrix : register(b0)
{
	matrix world;
}

cbuffer viewMatrix : register(b1)
{
	matrix view;
}
cbuffer projectionMatrix : register(b2)
{
	matrix projection;
}

struct VS_IN
{
	float3 Pos : SV_POSITION;
	float3 Normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_OUT
{
	float4 Pos : SV_POSITION;
	float4 Normal : NORMAL;
	float2 uv : TEXCOORD;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	output.Pos = mul(float4(input.Pos, 1.0f), mul(world, mul(view, projection)));
	output.Normal = mul(float4(input.Normal, 1.0f), mul(world, mul(view, projection)));
	output.uv = input.uv;
	return output;
}