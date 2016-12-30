#pragma once
#include "DirectXTemplatePCH.h"


using namespace DirectX;


class DX11Render
{
public:
	static DX11Render* Instance();
	void Update(float deltaTime);

	void Cleanup();

	void Render();
	int InitDirectX(HINSTANCE hInstance, BOOL vSync);
	const BOOL EnableVSync = true;
	bool LoadContent();

private:
	DX11Render();

	void UnloadContent();
	
	void Present(bool vSync);
	
	void Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil);

	static DX11Render* instance;



	// Direct3d device and swap chain.
	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;
	IDXGISwapChain* d3dSwapChain;

	// Render target view for the back buffer of the swap chain.
	ID3D11RenderTargetView* d3dRenderTargetView;
	// Depth/stencil view for use as a depth buffer.
	ID3D11DepthStencilView* d3dDepthStencilView;
	// A texture to assiciate to the depth stencil view.
	ID3D11Texture2D* d3dDepthStencilBuffer;

	// Define the functionality of the depth/stencil stages
	ID3D11DepthStencilState* d3dDepthStencilState;
	// Define the functionality of the rasterizer stage
	ID3D11RasterizerState* d3dRasterizerState;
	D3D11_VIEWPORT Viewport = { 0 };

	// Vertex buffer data
	ID3D11InputLayout* d3dInputLayout;
	ID3D11Buffer* d3dVertexBuffer;
	ID3D11Buffer* d3dIndexBuffer;

	// Shader data
	ID3D11VertexShader* d3dVertexShader;
	ID3D11PixelShader* d3dPixelShader;

	// Shader resources
	enum  ConstanBuffer
	{
		CB_Application,
		CB_Frame,
		CB_Object,
		NumConstantBuffers
	};

	ID3D11Buffer* d3dConstantBuffers[NumConstantBuffers];

	// Demo parameters
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;

};

struct VertexPosColor
{
	XMFLOAT3 Position;
	XMFLOAT3 Color;
};

template<typename T, std::size_t N = sizeof(T)>void ZeroMemory2(T* const t) { ::ZeroMemory(t, N); }

