#ifndef D3D_H
#define D3D_H

#include "SharedMemory.h"
#include "WICTextureLoader.h"

#define WINDOWSWIDTH 640.0f
#define WINDOWSHEIGHT 480.0f

class D3D : SharedMemory
{
public:

	D3D();
	D3D(HWND win);
	~D3D();

	void Update();
	void Render();

private:
	float windowWidth;
	float windowHeight;

	// Creators
	void Create();
	ID3D11Buffer* CreateMesh(size_t size, const void* data, size_t vertexCount);
	void CreateTexture(int lMesh);
	ID3D11Buffer* D3D::CreateConstantBuffer(size_t size, const void* data);
	void CreateShaders();

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

	// Shared memory
	//SharedMemory sm;
	unsigned int smType;
};

#endif