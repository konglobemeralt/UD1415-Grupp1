// Input data
struct VS_IN
{
	float3 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float4 boneWeight : BLENDWEIGHTS;
	unsigned int4 boneIndex : BLENDINDICES;
};

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

// Bones matrices
cbuffer Bones : register(b3)
{
	matrix bones[58];
};

// Output data
struct VS_OUT
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;
	float4 vertex = float4(input.pos, 1.0f);
	float4 animVertex;

	// Calculating the new position for every vertex based on the bones matrices
	animVertex = mul(vertex, bones[input.boneIndex.x]) * input.boneWeight.x;
	animVertex = mul(vertex, bones[input.boneIndex.y]) * input.boneWeight.y + animVertex;
	animVertex = mul(vertex, bones[input.boneIndex.z]) * input.boneWeight.z + animVertex;
	animVertex = mul(vertex, bones[input.boneIndex.w]) * input.boneWeight.w + animVertex;

	// Send it to output
	output.pos = mul(animVertex, mul(view, projection));
	output.uv = input.uv;
	//output.pos = mul(float4(input.pos, 1.0f), mul(world, mul(view, projection)));
	return output;
}