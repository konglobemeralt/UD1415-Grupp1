#include "SharedMemory.h"

SharedMemory::SharedMemory()
{
	//OpenMemory(1.0f / 256.0f);
	OpenMemory(100);
	slotSize = 256;
	view = new XMFLOAT4X4();
	projection = new XMFLOAT4X4();
}

SharedMemory::~SharedMemory()
{
	if (UnmapViewOfFile(cb) == 0)
		OutputDebugStringA("Failed unmap CircBuffer!");
	if (CloseHandle(fmCB) == 0)
		OutputDebugStringA("Failed close fmCB!");
	if (UnmapViewOfFile(buffer) == 0)
		OutputDebugStringA("Failed unmap buffer!");
	if (CloseHandle(fmMain) == 0)
		OutputDebugStringA("Failed unmap fmMain!");
}

void SharedMemory::OpenMemory(float size)
{
	size *= 1024 * 1024;
	memSize = size;
	// Circular buffer data
	fmCB = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		size,
		L"Global/CircularBuffer");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		OutputDebugStringA("CircularBuffer allready exist\n");

	if (fmCB == NULL)
		OutputDebugStringA("Could not open file mapping object! -> CircularBuffer\n");

	cb = (CircBuffer*)MapViewOfFile(fmCB, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (cb == NULL)
	{
		OutputDebugStringA("Could not map view of file!\n");
		CloseHandle(cb);
	}

	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		cb->head = 0;
		cb->tail = 0;
		cb->freeMem = size;
	}

	// Main data
	fmMain = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		size,
		L"Global/MainData");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		OutputDebugStringA("MainData allready exist\n");

	if (fmMain == NULL)
		OutputDebugStringA("Could not open file mapping object! -> MainData\n");

	buffer = MapViewOfFile(fmMain, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (buffer == NULL)
	{
		OutputDebugStringA("Could not map view of file!\n");
		CloseHandle(buffer);
	}
}

int SharedMemory::ReadMSGHeader()
{
	unsigned int type = 1;
	memcpy(&type, (char*)buffer, sizeof(int));
	return type;
}

void SharedMemory::ReadMemory(unsigned int type)
{
	//if (type == TMeshCreate)
	//{
	//	// Read and store whole mesh data

	//	// Size of mesh
	//	memcpy(&meshSize, (char*)buffer + localTail, sizeof(int));
	//	localTail += sizeof(int);

	//	meshes.push_back(MeshData());
	//	//meshes.back().transform = new XMFLOAT4X4();

	//	if (meshSize > 0)
	//	{
	//		meshes.back().vertexCount = meshSize;
	//		meshes.back().pos.resize(meshes.back().vertexCount);

	//		// Vertex data
	//		memcpy(meshes.back().pos.data(), (char*)buffer + localTail, sizeof(XMFLOAT3)* meshes.back().vertexCount);
	//		localTail += sizeof(XMFLOAT3)* meshes.back().vertexCount;

	//		// Move tail
	//		cb->freeMem += msgHeader.byteSize + localFreeMem;
	//		cb->tail += msgHeader.byteSize;
	//	}
	//}

	unsigned int tail = 0;
	//// Camera
	//memcpy(projection, (char*)buffer + tail, sizeof(XMFLOAT4X4));
	//tail += sizeof(XMFLOAT4X4);
	//memcpy(view, (char*)buffer + tail, sizeof(XMFLOAT4X4));
	//tail += sizeof(XMFLOAT4X4);

	meshes.push_back(MeshData());
	//DirectX::XMStoreFloat4x4(&meshes.back().transform, XMMatrixIdentity());
	meshes.back().pos.resize(4);
	meshes.back().indices.resize(6);
	meshes.back().boneIndices.resize(4*4);
	meshes.back().boneWeights.resize(4*4);
	// Vertices
	memcpy(meshes.back().pos.data(), (char*)buffer + tail, sizeof(XMFLOAT3) * 4);
	tail += sizeof(XMFLOAT3) * 4;
	// Indices
	memcpy(meshes.back().indices.data(), (char*)buffer + tail, sizeof(int) * 6);
	tail += sizeof(int) * 6;
	// Bone indices
	memcpy(meshes.back().boneIndices.data(), (char*)buffer + tail, sizeof(int) * 4*4);
	tail += sizeof(int) * 4*4;
	// Bone weights
	memcpy(meshes.back().boneWeights.data(), (char*)buffer + tail, sizeof(float) * 4*4);
	tail += sizeof(int) * 4*4;

	// Bone matrices
	memcpy(&meshes.back().transform, (char*)buffer + tail, sizeof(XMFLOAT4X4));
	tail += sizeof(XMFLOAT4X4);

	//XMMATRIX temp = XMLoadFloat4x4(&meshes.back().transform);
	//XMMATRIX rot = XMMatrixRotationZ(0.785398f);
	//temp = temp * rot;
	//XMStoreFloat4x4(&meshes.back().transform, rot);
}