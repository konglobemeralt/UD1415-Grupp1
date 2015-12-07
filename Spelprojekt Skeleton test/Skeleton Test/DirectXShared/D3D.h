#ifndef D3D_H
#define D3D_H

#include "WICTextureLoader.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <string>
#include <fstream>
#include <vector>
#include <fstream>

#define WINDOWSWIDTH 640.0f
#define WINDOWSHEIGHT 480.0f

using namespace DirectX;
using namespace std;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

class D3D
{
public:

	D3D();
	D3D(HWND win);
	~D3D();

	void Update();
	void Render();
	void UpdateBones(float time, unsigned int layer);

	bool createBool = false;

private:
	float windowWidth;
	float windowHeight;

	// Creators
	void Create();
	ID3D11Buffer* CreateMesh(size_t size, const void* data, size_t vertexCount);
	void CreateTexture();
	ID3D11Buffer* D3D::CreateConstantBuffer(size_t size, const void* data);
	void CreateShaders();
	void Interpolate(XMFLOAT4X4& matrix, float time, unsigned int boneIndex, unsigned int layer);

	// D3D
	IDXGISwapChain* swapChain;
	ID3D11Device* device;
	ID3D11DeviceContext* devcon;
	ID3D11RenderTargetView* backbuffer;
	ID3D11DepthStencilView* depthStencilView;
	D3D11_VIEWPORT viewPort;

	// Shaders
	ID3D11InputLayout* inputLayout;
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;

	// Camera
	ID3D11Buffer* cameraBuffers[2];
	XMFLOAT4X4 cameraMatrices[2];

	// Mesh
	struct MeshData
	{
		float pos[3];
		float uv[2];
		float weights[4];
		unsigned int indices[4];
	};
	vector<MeshData> meshData;
	ID3D11ShaderResourceView* texture;
	ID3D11Buffer* meshBuffer;
	XMFLOAT4X4 worldMatrix;
	ID3D11Buffer* worldBuffer;

	// Skeleton
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
		vector<int> parent;
		vector<XMFLOAT4X4> bindPose;
	}boneData;

	// Animation
	struct AnimationHeader
	{
		unsigned int version;
		unsigned int skeletonID;
		unsigned int framerate;
		unsigned int nrOfLayers;
	}animHeader;

	struct BoneAnimation
	{
		std::vector<Transform> tranform;
	};

	struct AnimationLayers
	{
		unsigned int nrOfKeys;
		std::vector<float> times;
		std::vector<unsigned int> keys;
		vector<BoneAnimation> bones;
	};
	vector<AnimationLayers> animLayer;

	// Interpolation
	vector<XMFLOAT4X4> toParentTransforms;
	vector<XMFLOAT4X4> toRootTransforms;
	vector<XMFLOAT4X4> finalTransforms;
	ID3D11Buffer* boneBuffer;
	D3D11_MAPPED_SUBRESOURCE subMapBones;
	float currTime, nextTime, lerpPercent;

	struct BoneStruct
	{
		XMFLOAT4X4 bone[50];
	}* boneStruct;
};

#endif