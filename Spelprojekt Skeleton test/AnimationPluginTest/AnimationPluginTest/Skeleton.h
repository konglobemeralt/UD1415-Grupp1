#pragma once
#include <vector>

class Animation
{
public:
	Animation();
	~Animation();

	struct SkeletonHeader
	{
		unsigned int version;
		unsigned int skeletonID;
		unsigned nrOfBones;
	}skelHead;

	struct Transform
	{
		XMFLOAT3 translation;
		XMFLOAT4 rotation;
		XMFLOAT3 scale;
	};

	struct BoneData
	{
		std::vector<int> parent;
		std::vector<XMFLOAT4X4> bindPose;
	}boneData;

	struct BoneAnimation
	{
		std::vector<Transform> tranform;
	};

	struct AnimationHeader
	{
		unsigned int version;
		unsigned int skeletonID;
		unsigned int framerate;
		unsigned int nrOfLayers;
	};

	struct AnimationLayers
	{
		unsigned int nrOfKeys;
		std::vector<float> times;
		std::vector<unsigned int> keys;
		vector<BoneAnimation> bones;
		MObject layerObject;
	};

	void GetBindPose(MObject& object, unsigned int index);
	void GetAnimationLayer();
	void GetAnimation();
	void WriteAnimationData();

	MStringArray boneNames;
	string fileName;
	std::vector<AnimationLayers> animLayer;
	AnimationHeader animHeader;
};

Animation::Animation() {};
Animation::~Animation() {};