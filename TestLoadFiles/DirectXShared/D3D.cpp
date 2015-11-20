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

void D3D::Update()
{
	ReadMemory(0);

	//// Camera
	//projection->_11 = projection->_22 / (windowWidth / windowHeight);
	//viewMatrix = CreateConstantBuffer(sizeof(XMFLOAT4X4), view);
	//projectionMatrix = CreateConstantBuffer(sizeof(XMFLOAT4X4), projection);

	//// Indices
	//D3D11_BUFFER_DESC cbDesc;
	//cbDesc.ByteWidth = sizeof(int) * 6;
	//cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	//cbDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cbDesc.MiscFlags = 0;
	//cbDesc.StructureByteStride = 0;

	//D3D11_SUBRESOURCE_DATA cbInitData;
	//cbInitData.pSysMem = meshes.back().indices.data();
	//cbInitData.SysMemPitch = 0;
	//cbInitData.SysMemSlicePitch = 0;
	//device->CreateBuffer(&cbDesc, &cbInitData, &meshes.back().meshesBuffer[0]);

	createBool = true;
}

void D3D::Render()
{
	// clear the back buffer to a deep blue
	float clearColor[] = { 0, 0, 1, 1 };
	devcon->ClearRenderTargetView(backbuffer, clearColor);
	devcon->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devcon->VSSetConstantBuffers(1, 1, &viewMatrix);
	devcon->VSSetConstantBuffers(2, 1, &projectionMatrix);
	devcon->IASetInputLayout(inputLayout);
	devcon->VSSetShader(vertexShader, NULL, 0);
	devcon->PSSetShader(pixelShader, NULL, 0);

	//unsigned int strides[3] = { 12, 8, 12 };
	//unsigned int offsets[3] = { 0, 0, 0 };

	unsigned int strides[3] = { 12, 12, 8 };
	unsigned int offsets[3] = { 0, 0, 0 };

	for (size_t i = 0; i < meshes.size(); i++)
	{
		if (meshes[i].meshesBuffer[0] != NULL)
		{
			devcon->PSSetShaderResources(0, 1, &meshes[i].meshTextures);
			devcon->VSSetConstantBuffers(0, 1, &meshes[i].transformBuffer);
			devcon->IASetVertexBuffers(0, 3, meshes[i].meshesBuffer, strides, offsets);
			devcon->Draw(meshes[i].pos.size(), 0);
		}
	}
	swapChain->Present(0, 0);
}

void D3D::Create()
{
	// CAMERA
	XMStoreFloat4x4(view, XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0.0f, 2.0f, -2.0f, 1.0f), XMVectorSet(0.0f, -1.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))));
	XMStoreFloat4x4(projection, XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PI * 0.45f, 640.0f / 480.0f, 1.0f, 1000.0f)));
	projection->_11 = projection->_22 / (windowWidth / windowHeight);
	viewMatrix = CreateConstantBuffer(sizeof(XMFLOAT4X4), view);
	projectionMatrix = CreateConstantBuffer(sizeof(XMFLOAT4X4), projection);


	// Load bin file
	std::ifstream infile;
	infile.open("C:/New folder/pCube1.bin", std::ifstream::binary);

	if (infile.is_open() == true)
	{
		// Main header
		MainHeader mainHeader;
		infile.read((char*)&mainHeader, sizeof(MainHeader));

		// Mesh header
		Mesh mesh;
		vector<SubMesh> submesh;
		infile.read((char*)&mesh.header, sizeof(MeshHeader));

		// Mesh data
		mesh.geometry.points.resize(mesh.header.numberPoints);
		mesh.geometry.normals.resize(mesh.header.numberNormals);
		mesh.geometry.texCoords.resize(mesh.header.numberCoords);
		infile.read((char*)mesh.Name.data(), mesh.header.nameLength);
		infile.read((char*)mesh.geometry.points.data(), mesh.header.numberPoints * sizeof(Point));
		infile.read((char*)mesh.geometry.normals.data(), mesh.header.numberNormals * sizeof(Normal));
		infile.read((char*)mesh.geometry.texCoords.data(), mesh.header.numberCoords * sizeof(TexCoord));
		submesh.resize(mainHeader.meshCount);

		mesh.geometry.faces.resize(mesh.header.numberFaces);
		for (size_t i = 0; i < mesh.header.numberFaces; i++){
			for (size_t j = 0; j < 3; j++) {
				infile.read((char*)&mesh.geometry.faces[i].verts[j].pointID, 4);
				infile.read((char*)&mesh.geometry.faces[i].verts[j].normalID, 4);
				infile.read((char*)&mesh.geometry.faces[i].verts[j].texCoordID, 4);
			}
		}

		for (size_t i = 0; i < mainHeader.meshCount; i++){
			// Mesh header
			
			infile.read((char*)&submesh[i].header, sizeof(MeshHeader));

			// Mesh data
			mesh.subMeshes[i].geometry.points.resize(submesh[i].header.numberPoints);
			mesh.subMeshes[i].geometry.normals.resize(submesh[i].header.numberNormals);
			mesh.subMeshes[i].geometry.texCoords.resize(submesh[i].header.numberCoords);
			infile.read((char*)mesh.subMeshes[i].Name.data(), submesh[i].header.nameLength);
			infile.read((char*)mesh.subMeshes[i].geometry.points.data(), submesh[i].header.numberPoints * sizeof(Point));
			infile.read((char*)mesh.subMeshes[i].geometry.normals.data(), submesh[i].header.numberNormals * sizeof(Normal));
			infile.read((char*)mesh.subMeshes[i].geometry.texCoords.data(), submesh[i].header.numberCoords * sizeof(TexCoord));

			mesh.geometry.faces.resize(submesh[i].header.numberFaces);
			for (size_t j = 0; j < submesh[i].header.numberFaces; j++) {
				for (size_t k = 0; k < 3; k++) {
					infile.read((char*)&submesh[i].geometry.faces[j].verts[k].pointID, 4);
					infile.read((char*)&submesh[i].geometry.faces[j].verts[k].normalID, 4);
					infile.read((char*)&submesh[i].geometry.faces[j].verts[k].texCoordID, 4);
				}
			}
		}

		MatHeader matHeader;
		infile.read((char*)&matHeader, sizeof(MatHeader));
		infile.read((char*)&mesh.material.diffColor, 16);
		//infile.read((char*)mesh.material.diffuseTexture.data(), matHeader.diffuseNameLength);		// Crash here on reading of string.

		infile.read((char*)&mesh.material.specColor, 16);
		infile.read((char*)mesh.material.specularTexture.data(), matHeader.specularNameLength);



		// TEST TO PUT THE OBJERCT TOGHETER
		meshes.push_back(MeshData());
		meshes.back().pos.resize(mesh.header.numberFaces * 3);
		meshes.back().normal.resize(mesh.header.numberFaces * 3);
		meshes.back().uv.resize(mesh.header.numberFaces * 3);
		size_t index = 0;
		for (size_t i = 0; i < mesh.header.numberFaces; i++){
			for (size_t j = 0; j < 3; j++) {
				meshes.back().pos[index].x = mesh.geometry.points[mesh.geometry.faces[i].verts[j].pointID].x;
				meshes.back().pos[index].y = mesh.geometry.points[mesh.geometry.faces[i].verts[j].pointID].y;
				meshes.back().pos[index].z = mesh.geometry.points[mesh.geometry.faces[i].verts[j].pointID].z;
				meshes.back().normal[index] = mesh.geometry.normals[mesh.geometry.faces[i].verts[j].normalID];
				meshes.back().uv[index] = mesh.geometry.texCoords[mesh.geometry.faces[i].verts[j].texCoordID];
				index++;
			}
		}

		// Vertices
		meshes.back().meshesBuffer[0] = CreateMesh(sizeof(XMFLOAT3) * meshes.back().pos.size(), meshes.back().pos.data(), meshes.back().pos.size());
		meshes.back().meshesBuffer[1] = CreateMesh(sizeof(XMFLOAT3) * meshes.back().normal.size(), meshes.back().normal.data(), meshes.back().normal.size());
		meshes.back().meshesBuffer[2] = CreateMesh(sizeof(XMFLOAT2) * meshes.back().uv.size(), meshes.back().uv.data(), meshes.back().uv.size());

		// Transform
		DirectX::XMStoreFloat4x4(&meshes.back().transform, XMMatrixIdentity());
		meshes.back().transformBuffer = CreateConstantBuffer(sizeof(XMFLOAT4X4), &meshes.back().transform);

		// Texture
		CreateTexture(0);
	}
	infile.close();
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

void D3D::CreateTexture(int lMesh)
{
	CoInitialize(NULL);
	wstring name = L"D:/Group-project-level-editor/CubeTexture.png";

	CreateWICTextureFromFile(device, name.c_str(), NULL, &meshes.back().meshTextures);
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
	D3DCompileFromFile(L"VertexShader.hlsl", NULL, NULL, "main", "vs_5_0", 0, NULL, &pVS, NULL);
	device->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &vertexShader);

	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	device->CreateInputLayout(inputDesc, 3, pVS->GetBufferPointer(), pVS->GetBufferSize(), &inputLayout);
	pVS->Release();

	//create pixel shader
	ID3DBlob* pPS = nullptr;
	D3DCompileFromFile(L"PixelShader.hlsl", NULL, NULL, "main", "ps_5_0", 0, NULL, &pPS, NULL);
	device->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &pixelShader);
	pPS->Release();
}