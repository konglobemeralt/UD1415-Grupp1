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
	if (cb->freeMem < memSize)
	{
		// Sets tail to 0 if there are no place to read
		if (cb->tail == memSize)
			cb->tail = 0;

		localTail = cb->tail;
		localFreeMem = 0;

		// Message header
		memcpy(&msgHeader, (char*)buffer + localTail, sizeof(MSGHeader));
		localTail += sizeof(MSGHeader);

		// Check if there are something to read else move tail to 0
		if (msgHeader.type > TAmount)
		{
			localFreeMem += (memSize - cb->tail);
			cb->tail = 0;
			localTail = cb->tail;

			// Read message header again at 0
			memcpy(&msgHeader, (char*)buffer + localTail, sizeof(MSGHeader));
			localTail += sizeof(MSGHeader);
		}

		return msgHeader.type;
	}
	return -1;
}

void SharedMemory::ReadMemory(unsigned int type)
{
	if (type == TMeshCreate)
	{
		// Read and store whole mesh data

		// Size of mesh
		memcpy(&meshSize, (char*)buffer + localTail, sizeof(int));
		localTail += sizeof(int);

		meshes.push_back(MeshData());
		//meshes.back().transform = new XMFLOAT4X4();

		if (meshSize > 0)
		{
			meshes.back().vertexCount = meshSize;

			// Rezise to hold every vertex
			//meshes.back().pos = new XMFLOAT3[meshes.back().vertexCount];
			//meshes.back().uv = new XMFLOAT2[meshes.back().vertexCount];
			//meshes.back().normal = new XMFLOAT3[meshes.back().vertexCount];
			meshes.back().pos.resize(meshes.back().vertexCount);
			meshes.back().uv.resize(meshes.back().vertexCount);
			meshes.back().normal.resize(meshes.back().vertexCount);

			// Vertex data
			memcpy(meshes.back().pos.data(), (char*)buffer + localTail, sizeof(XMFLOAT3)* meshes.back().vertexCount);
			localTail += sizeof(XMFLOAT3)* meshes.back().vertexCount;
			memcpy(meshes.back().uv.data(), (char*)buffer + localTail, sizeof(XMFLOAT2)* meshes.back().vertexCount);
			localTail += sizeof(XMFLOAT2)* meshes.back().vertexCount;
			memcpy(meshes.back().normal.data(), (char*)buffer + localTail, sizeof(XMFLOAT3)* meshes.back().vertexCount);
			localTail += sizeof(XMFLOAT3)* meshes.back().vertexCount;

			// Matrix data
			memcpy(&meshes.back().transform, (char*)buffer + localTail, sizeof(XMFLOAT4X4));
			localTail += sizeof(XMFLOAT4X4);

			// Material data
			memcpy(&meshes.back().meshTex.materialColor, (char*)buffer + localTail, sizeof(XMFLOAT4));
			localTail += sizeof(XMFLOAT4);

			//Texture true or false
			memcpy(&meshes.back().meshTex.textureExist.x, (char*)buffer + localTail, sizeof(int));
			localTail += sizeof(int);

			if (meshes.back().meshTex.textureExist.x == 1)
			{
				//Texture path size
				memcpy(&meshes.back().textureSize, (char*)buffer + localTail, sizeof(int));
				localTail += sizeof(int);

				meshes.back().texturePath = new char[meshes.back().textureSize + 1];

				//Texture data
				memcpy(meshes.back().texturePath, (char*)buffer + localTail, meshes.back().textureSize);
				localTail += meshes.back().textureSize;

				meshes.back().texturePath[meshes.back().textureSize] = '\0';

			}

			// Move tail
			cb->freeMem += msgHeader.byteSize + localFreeMem;
			cb->tail += msgHeader.byteSize;
		}
	}
}