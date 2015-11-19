// Input data
struct VS_IN
{
	float3 pos : SV_POSITION;
	float4 boneWeight : BLENDWEIGHTS;
	float4 boneIndex : BLENDINDICES;
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
	float4x4 bones[58];
};

// Output data
struct VS_OUT
{
	float4 pos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;
	float4 vertex = float4(input.pos, 1.0f);
	float4 animVertex;

	// Calculating the new position for every vertex based on the bones matrices
	// Calculating for bone 1
	int boneIndex = int(input.boneIndex.x);
	animVertex = mul(vertex, bones[boneIndex]) * input.boneWeight.x;

	// Calculating for bone 2
	boneIndex = int(input.boneIndex.y);
	animVertex = mul(vertex, bones[boneIndex]) * input.boneWeight.y + animVertex;

	// Calculating for bone 3
	boneIndex = int(input.boneIndex.z);
	animVertex = mul(vertex, bones[boneIndex]) * input.boneWeight.z + animVertex;

	// Calculating for bone 4
	boneIndex = int(input.boneIndex.w);
	animVertex = mul(vertex, bones[boneIndex]) * input.boneWeight.w + animVertex;

	// Send it to output
	//output.pos = animVertex;
	output.pos = mul(float4(input.pos, 1.0f), mul(world, mul(view, projection)));
	return output;
}