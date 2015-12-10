#pragma once
#include <vector>
#include <DirectXMath.h>
#include <string>

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

class Animation
{
public:
	Animation();
	~Animation();

	struct AnimationHeader
	{
		unsigned int version;
		unsigned int skeletonID;
		unsigned int framerate;
		unsigned int nrOfLayers;
		unsigned nrOfBones;
	};

	struct BindPoseData
	{
		int parent;
		DirectX::XMFLOAT4X4 bindPose;
	};

	struct FrameData
	{
		DirectX::XMFLOAT3 translation;
		DirectX::XMFLOAT4 rotation;
		DirectX::XMFLOAT3 scale;
	};

	struct BoneAnimation
	{
		std::vector<FrameData> tranform;
	};

	struct AnimationLayers
	{
		unsigned int nrOfFrames;
		std::vector<float> time;
		std::vector<int> key;
		std::vector<BoneAnimation> bones;
		MObject layerObject;
	};

	MStringArray boneNames;
	std::string fileName;
	std::vector<AnimationLayers> animLayer;
	AnimationHeader animHeader;
	std::vector<BindPoseData> bindPose;
	
};

Animation::Animation() {};
Animation::~Animation() {};