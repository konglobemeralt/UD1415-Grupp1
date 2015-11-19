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

	// CAMERA
	XMFLOAT4X4* view;
	XMFLOAT4X4* projection;
	XMFLOAT4X4 projectionTemp;
	ID3D11Buffer* viewMatrix;
	ID3D11Buffer* projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mapSub;
	// _________________________________________________________________________________________________

	struct Point
	{
		float x, y, z;
		int boneIndices[4];
		float boneWeigths[4];
	};

	struct Normal
	{
		float x, y, z;
	};


	struct TexCoord
	{
		float u, v;
	};

	struct Vertex
	{
		int pointID, normalID, texCoordID;
	};

	struct Face
	{
		Vertex verts[3];
	};

	struct Material
	{
		float diffColor[4], specColor[4];
		std::string diffuseTexture, specularTexture;
	};

	struct PointLight
	{
		float pos[3], col[3], intensity, dropoff;
	};

	struct SpotLight
	{
		float pos[3], col[3], intensity, angle, direction[3], dropoff;
	};

	struct Geometry
	{
		std::vector<Point> points;
		std::vector<Normal> normals;
		std::vector<TexCoord> texCoords;
		std::vector<Face> faces;

		std::vector<PointLight> pointLights;
		std::vector<SpotLight> spotLights;
	};

	struct SubMesh
	{
		std::string Name;
		Geometry geometry;
	};

	struct Mesh
	{
		std::string Name;
		int skeletonID;
		std::vector<SubMesh> subMeshes;
		Geometry geometry;
		Material material;
	};

	// Mesh headers
	struct MainHeader {
		int version, meshCount;
	};

	struct MeshHeader
	{
		int nameLength, numberPoints, numberNormals, numberCoords, numberFaces, subMeshID, numberPointLights, numberSpotLights;
	};

	struct MatHeader {
		int diffuseNameLength, specularNameLength;
	};

	struct Point2
	{
		float x, y, z;
	};
	struct MeshData
	{
		//VertexData* vertexData;
		vector<int> indices;
		vector<Point2> pos;
		vector<TexCoord> uv;
		vector<Normal> normal;
		vector<int> boneIndices;
		vector<float> boneWeights;
		unsigned int vertexCount;
		XMFLOAT4X4 transform;
		ID3D11Buffer* meshesBuffer[4];
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
};

#endif