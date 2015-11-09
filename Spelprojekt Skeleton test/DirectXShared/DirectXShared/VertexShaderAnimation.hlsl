// Input data
struct VS_IN
{
	float3 pos : SV_POSITION;
	float4 bw : BLENDWEIGHTS;
	float4 bi : BLENDINDICES;
};

// Bones matrices
cbuffer Bones
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
	int boneIndex = int(input.bi.x);
	animVertex = mul(vertex, bones[boneIndex]) * input.bw.x;

	// Calculating for bone 2
	boneIndex = int(input.bi.y);
	animVertex = mul(vertex, bones[boneIndex]) * input.bw.y + animVertex;

	// Calculating for bone 3
	boneIndex = int(input.bi.z);
	animVertex = mul(vertex, bones[boneIndex]) * input.bw.z + animVertex;

	// Calculating for bone 4
	boneIndex = int(input.bi.w);
	animVertex = mul(vertex, bones[boneIndex]) * input.bw.w + animVertex;

	// Send it to output
	output.pos = animVertex;
	return output;
}