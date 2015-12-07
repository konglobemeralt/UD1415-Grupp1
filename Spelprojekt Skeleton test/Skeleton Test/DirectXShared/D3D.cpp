#include "D3D.h"

D3D::D3D() {}

D3D::D3D(HWND win)
{
	windowWidth = WINDOWSWIDTH;
	windowHeight = WINDOWSHEIGHT;
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = win;                           // the window to be used
	scd.SampleDesc.Count = 1;                               // how many multisamples
	scd.SampleDesc.Quality = 0;
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

															// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&swapChain,
		&device,
		NULL,
		&devcon);

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		device->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
		pBackBuffer->Release();

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.Width = 640;
		depthStencilDesc.Height = 480;
		depthStencilDesc.MipLevels = 0;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.MiscFlags = 0;
		depthStencilDesc.CPUAccessFlags = 0;

		ID3D11Texture2D* pStencilTexture;
		device->CreateTexture2D(&depthStencilDesc, NULL, &pStencilTexture);
		device->CreateDepthStencilView(pStencilTexture, NULL, &depthStencilView);
		pStencilTexture->Release();

		// set the render target as the back buffer
		devcon->OMSetRenderTargets(1, &backbuffer, depthStencilView);

		ID3D11RasterizerState* p_RasterState;					// disables default backface culling
		D3D11_RASTERIZER_DESC descRaster;
		ZeroMemory(&descRaster, sizeof(D3D11_RASTERIZER_DESC));
		descRaster.FillMode = D3D11_FILL_SOLID;					// WIREFRAME;
		descRaster.CullMode = D3D11_CULL_NONE;
		descRaster.MultisampleEnable = TRUE;

		device->CreateRasterizerState(&descRaster, &p_RasterState);
		devcon->RSSetState(p_RasterState);
	}

	viewPort.Width = (float)WINDOWSWIDTH;
	viewPort.Height = (float)WINDOWSHEIGHT;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	devcon->RSSetViewports(1, &viewPort);

	CreateShaders();
	Create();
}

D3D::~D3D() {}

void D3D::Render()
{
	// clear the back buffer to a deep blue
	float clearColor[] = { 0, 0, 1, 1 };
	devcon->ClearRenderTargetView(backbuffer, clearColor);
	devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->VSSetConstantBuffers(1, 1, &cameraBuffers[0]);
	devcon->VSSetConstantBuffers(2, 1, &cameraBuffers[1]);
	devcon->IASetInputLayout(inputLayout);
	devcon->VSSetShader(vertexShader, NULL, 0);
	devcon->PSSetShader(pixelShader, NULL, 0);
	devcon->PSSetShaderResources(0, 1, &texture);

	//unsigned int strides[3] = { 12, 8, 12 };
	//unsigned int offsets[3] = { 0, 0, 0 };

	unsigned int strides[1] = { 52 };
	unsigned int offsets[1] = { 0 };

	devcon->VSSetConstantBuffers(0, 1, &worldBuffer);
	devcon->IASetVertexBuffers(0, 1, &meshBuffer, strides, offsets);
	devcon->Draw(meshData.size(), 0);
	
	swapChain->Present(0, 0);
}

void D3D::Create()
{
	// CAMERA
	XMStoreFloat4x4(&cameraMatrices[0], XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(2.0f, 2.0f, -2.0f, 0.0f), XMVectorSet(-1.0f, 0.0f, 1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	XMStoreFloat4x4(&cameraMatrices[1], XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PI * 0.45f, 640.0f / 480.0f, 1.0f, 1000.0f)));
	cameraMatrices[1]._11 = cameraMatrices[1]._22 / (windowWidth / windowHeight);
	cameraBuffers[0] = CreateConstantBuffer(sizeof(XMFLOAT4X4), &cameraMatrices[0]);
	cameraBuffers[1] = CreateConstantBuffer(sizeof(XMFLOAT4X4), &cameraMatrices[1]);
	CreateTexture();

	string name = "basemanBody";
	string fileDir = "C:/Users/Spelprojekt/Desktop/Spelprojekt Skeleton test/Files/";
	// Mesh
	ifstream infile;
	infile.open(fileDir + name + "_Mesh.bin", std::ofstream::binary);
	if (infile) {
		unsigned int meshSize = 0;
		infile.read((char*)&meshSize, sizeof(int));
		meshData.resize(meshSize);
		infile.read((char*)meshData.data(), sizeof(MeshData) * meshSize);
		infile.close();

		meshBuffer = CreateMesh(sizeof(MeshData) * meshData.size(), meshData.data(), meshData.size());
		XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
		worldBuffer = CreateConstantBuffer(sizeof(XMFLOAT4X4), &worldMatrix);
	}

	// Skeleton
	infile.open(fileDir + name + "_Skeleton.bin", std::ofstream::binary);
	if (infile) {
		// Skeleton header
		infile.read((char*)&skelHead, sizeof(SkeletonHeader));

		// Parents and bindposes
		boneData.parent.resize(skelHead.nrOfBones);
		boneData.bindPose.resize(skelHead.nrOfBones);
		infile.read((char*)boneData.parent.data(), sizeof(int) * skelHead.nrOfBones);
		infile.read((char*)boneData.bindPose.data(), sizeof(XMFLOAT4X4) * skelHead.nrOfBones);
		infile.close();
	}

	// Animation
	infile.open(fileDir + name + "_Animation.bin", std::ofstream::binary);
	if (infile) {
		// Animation header
		infile.read((char*)&animHeader, sizeof(AnimationHeader));

		// Number of keys
		animLayer.resize(animHeader.nrOfLayers);
		for (size_t i = 0; i < animHeader.nrOfLayers; i++){
			infile.read((char*)&animLayer[i].nrOfKeys, sizeof(int));
		}

		// Keys and transforms
		for (size_t i = 0; i < animHeader.nrOfLayers; i++) {
			animLayer[i].times.resize(animLayer[i].nrOfKeys);
			animLayer[i].keys.resize(animLayer[i].nrOfKeys);
			infile.read((char*)animLayer[i].times.data(), sizeof(float) * animLayer[i].nrOfKeys);
			infile.read((char*)animLayer[i].keys.data(), sizeof(int) * animLayer[i].nrOfKeys);
			animLayer[i].bones.resize(skelHead.nrOfBones);

			for (size_t j = 0; j < skelHead.nrOfBones; j++) {
				animLayer[i].bones[j].tranform.resize(animLayer[i].nrOfKeys);
				infile.read((char*)animLayer[i].bones[j].tranform.data(), sizeof(Transform) * animLayer[i].nrOfKeys);
			}
		}
		infile.close();

		// Fix buffers to interpolation
		toParentTransforms.resize(skelHead.nrOfBones);
		toRootTransforms.resize(skelHead.nrOfBones);
		finalTransforms.resize(skelHead.nrOfBones);
		boneStruct = new BoneStruct[skelHead.nrOfBones];

		D3D11_BUFFER_DESC constantBufferBonesDesc;
		memset(&constantBufferBonesDesc, 0, sizeof(constantBufferBonesDesc));
		constantBufferBonesDesc.ByteWidth = sizeof(BoneStruct) * skelHead.nrOfBones;
		constantBufferBonesDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantBufferBonesDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBufferBonesDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		device->CreateBuffer(&constantBufferBonesDesc, nullptr, &boneBuffer);
	}
}

void D3D::Interpolate(XMFLOAT4X4& matrix, float time, unsigned int boneIndex, unsigned int layer)
{
	if (time <= animLayer[layer].times[0])
	{
		XMVECTOR s = XMLoadFloat3(&animLayer[layer].bones[boneIndex].tranform[0].scale);
		XMVECTOR p = XMLoadFloat3(&animLayer[layer].bones[boneIndex].tranform[0].translation);
		XMVECTOR q = XMLoadFloat4(&animLayer[layer].bones[boneIndex].tranform[0].rotation);
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMStoreFloat4x4(&matrix, XMMatrixAffineTransformation(s, zero, q, p));
	}

	else if (time >= animLayer[layer].times[animLayer[layer].nrOfKeys - 1])
	{
		XMVECTOR s = XMLoadFloat3(&animLayer[layer].bones[boneIndex].tranform[animLayer[layer].nrOfKeys - 1].scale);
		XMVECTOR p = XMLoadFloat3(&animLayer[layer].bones[boneIndex].tranform[animLayer[layer].nrOfKeys - 1].translation);
		XMVECTOR q = XMLoadFloat4(&animLayer[layer].bones[boneIndex].tranform[animLayer[layer].nrOfKeys - 1].rotation);
		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMStoreFloat4x4(&matrix, XMMatrixAffineTransformation(s, zero, q, p));
	}

	else
	{
		for (int i = 0; i <= animLayer[layer].nrOfKeys; i++)
		{
			currTime = animLayer[layer].times[i];
			nextTime = animLayer[layer].times[i + 1];
			if (time >= currTime && time <= nextTime)
			{
				lerpPercent = (time - currTime) / (nextTime - currTime);


				XMVECTOR s0 = XMLoadFloat3(&animLayer[layer].bones[boneIndex].tranform[i].scale);
				XMVECTOR s1 = XMLoadFloat3(&animLayer[layer].bones[boneIndex].tranform[i + 1].scale);

				XMVECTOR p0 = XMLoadFloat3(&animLayer[layer].bones[boneIndex].tranform[i].translation);
				XMVECTOR p1 = XMLoadFloat3(&animLayer[layer].bones[boneIndex].tranform[i + 1].translation);

				XMVECTOR q0 = XMLoadFloat4(&animLayer[layer].bones[boneIndex].tranform[i].rotation);
				XMVECTOR q1 = XMLoadFloat4(&animLayer[layer].bones[boneIndex].tranform[i + 1].rotation);

				XMVECTOR s = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR p = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR q = XMQuaternionSlerp(q0, q1, lerpPercent);
				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&matrix, XMMatrixAffineTransformation(s, zero, q, p));

				break;
			}
		}
	}
}

void D3D::UpdateBones(float time, unsigned int layer)
{
	for (int i = 0; i < skelHead.nrOfBones; i++)
	{
		Interpolate(toParentTransforms[i], time, i, layer);
	}

	toRootTransforms[0] = toParentTransforms[0];

	for (int i = 1; i < skelHead.nrOfBones; i++)
	{
		// Current bone transform relative to its parent
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]); // Bone[i] toParentTransform
																	// Current bone parent tramnsform relative to Root 
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[boneData.parent[i]]); // Parent Bone[i] toRootTransform
		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);
		XMStoreFloat4x4(&toRootTransforms[i], toRoot); // toRootTransforms[i] = ToRoot
	}

	for (int i = 0; i < skelHead.nrOfBones; i++)
	{
		XMMATRIX offset = XMLoadFloat4x4(&boneData.bindPose[i]); // Inverse Bindpose
		XMMATRIX toRoot = XMLoadFloat4x4(&toRootTransforms[i]);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixMultiply(offset, toRoot)); // Store final matrix
	}
}

void D3D::Update()
{
	devcon->Map(boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subMapBones);
	boneStruct = (BoneStruct*)subMapBones.pData;

	for (int i = 0; i < skelHead.nrOfBones; i++){
		XMStoreFloat4x4(&boneStruct->bone[i], XMMatrixTranspose(XMLoadFloat4x4(&finalTransforms[i])));
	}

	devcon->Unmap(boneBuffer, 0);
	devcon->VSSetConstantBuffers(3, 1, &boneBuffer);
}

ID3D11Buffer* D3D::CreateMesh(size_t size, const void* data, size_t vertexCount)
{
	ID3D11Buffer* vertexBuffer;

	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.ByteWidth = size;

	D3D11_SUBRESOURCE_DATA VBdata;
	VBdata.pSysMem = data;
	VBdata.SysMemPitch = 0;
	VBdata.SysMemSlicePitch = 0;

	device->CreateBuffer(&vertexBufferDesc, &VBdata, &vertexBuffer);
	return vertexBuffer;
}

void D3D::CreateTexture()
{
	CoInitialize(NULL);
	wstring textureName = L"texturebrown.png";

	CreateWICTextureFromFile(device, textureName.c_str(), NULL, &texture);
}

ID3D11Buffer* D3D::CreateConstantBuffer(size_t size, const void* data)
{
	ID3D11Buffer* constantBuffer;

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = size;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA cbInitData;
	cbInitData.pSysMem = data;
	cbInitData.SysMemPitch = 0;
	cbInitData.SysMemSlicePitch = 0;
	HRESULT hr;
	hr = device->CreateBuffer(&cbDesc, &cbInitData, &constantBuffer);
	return constantBuffer;
}

void D3D::CreateShaders()
{
	//create vertex shader
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(L"VertexShaderAnimation.hlsl", NULL, NULL, "main", "vs_5_0", 0, NULL, &pVS, NULL);
	device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &vertexShader);

	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	device->CreateInputLayout(inputDesc, 4, pVS->GetBufferPointer(), pVS->GetBufferSize(), &inputLayout);
	pVS->Release();

	//create pixel shader
	ID3DBlob* pPS = nullptr;
	D3DCompileFromFile(L"PixelShader.hlsl", NULL, NULL, "main", "ps_5_0", 0, NULL, &pPS, NULL);
	device->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &pixelShader);
	pPS->Release();
}