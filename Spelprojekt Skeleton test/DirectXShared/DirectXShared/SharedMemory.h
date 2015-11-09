#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>
#include "Enumerations.h"
#include <fstream>

using namespace DirectX;
using namespace std;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

using namespace DirectX;

class SharedMemory
{
public:

	SharedMemory();
	~SharedMemory();

	void OpenMemory(float size);
	int ReadMSGHeader();
	void ReadMemory(unsigned int type);

	// SHARED MEOMRY
	HANDLE fmCB;
	HANDLE fmMain;
	unsigned int slotSize;
	unsigned int localTail;

	struct CircBuffer
	{
		unsigned int freeMem;
		unsigned int head;
		unsigned int tail;
	}*cb;
	unsigned int localFreeMem;

	size_t memSize;
	void* buffer;

	// MESSAGE HEADER
	struct MSGHeader
	{
		unsigned int type;
		unsigned int byteSize;
	}msgHeader;

	// MESH
	struct meshTexture
	{
		XMINT4 textureExist;
		XMFLOAT4 materialColor;
	};
	struct MeshData
	{
		//VertexData* vertexData;
		vector<XMFLOAT3> pos;
		vector<XMFLOAT2> uv;
		vector<XMFLOAT3> normal;
		unsigned int vertexCount;
		XMFLOAT4X4 transform;
		ID3D11Buffer* meshesBuffer[3];
		ID3D11Buffer* transformBuffer;

		//Material:
		ID3D11Buffer* colorBuffer;
		meshTexture meshTex;
		ID3D11ShaderResourceView* meshTextures;

		//Texture:
		unsigned int textureSize;
		char* texturePath;
	};
	vector<MeshData> meshes;
	unsigned int localMesh;
	unsigned int localVertex;
	XMFLOAT3 vtxChanged;
	unsigned int meshSize;

	// CAMERA
	XMFLOAT4X4* view;
	XMFLOAT4X4* projection;
	XMFLOAT4X4 projectionTemp;
	ID3D11Buffer* viewMatrix;
	ID3D11Buffer* projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mapSub;
};

#endif