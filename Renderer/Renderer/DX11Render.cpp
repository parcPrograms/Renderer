#include "DirectXTemplatePCH.h"
#include "DX11Render.h"
#include "Window.h"


DX11Render* DX11Render::instance = 0;

DX11Render* DX11Render::Instance() {
	if (instance == 0) {
		instance = new DX11Render();
	}
	return instance;
}

DX11Render::DX11Render() {
	d3dDevice = nullptr;
	d3dDeviceContext = nullptr;
	d3dSwapChain = nullptr;
	d3dRenderTargetView = nullptr;
	d3dDepthStencilView = nullptr;
	d3dDepthStencilBuffer = nullptr;
	d3dDepthStencilState = nullptr;
	d3dRasterizerState = nullptr;
	Viewport = { 0 };
	d3dInputLayout = nullptr;
	d3dVertexBuffer = nullptr;
	d3dIndexBuffer = nullptr;
	d3dVertexShader = nullptr;
	d3dPixelShader = nullptr;
}

/**
* INitalize the DirectX device and swap chain.
*/
int DX11Render::InitDirectX(HINSTANCE hInstance, BOOL vSync)
{
	// A window handle must have been created already.
	assert(Window::Instance()->GetWindowHandle() != 0);

	RECT clientRect;
	GetClientRect(Window::Instance()->GetWindowHandle(), &clientRect);

	// Compute the exact client dimensions. this will be use
	// to initializ the render targets for our swap chain.
	unsigned int clientWidth = clientRect.right - clientRect.left;
	unsigned int clientHeight = clientRect.bottom - clientRect.top;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	DXGI_RATIONAL DxgiRational; DxgiRational.Numerator = 0; DxgiRational.Denominator = 1; //hardcoding in a 0/1 here because the refresrate function isnt real
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = clientWidth;
	swapChainDesc.BufferDesc.Height = clientHeight;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate = DxgiRational;// QueryRefreshRate(clientWidth, clientHeight, vSync);
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = Window::Instance()->GetWindowHandle();
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = TRUE;

	UINT createDeviceFlags = 0;
#if _DEBUG
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	// these are the feature levels that we will accept.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	//this will be the feature level that
	//is used to create out device and swap chain
	D3D_FEATURE_LEVEL featureLevel;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, createDeviceFlags, featureLevels, _countof(featureLevels),
		D3D11_SDK_VERSION, &swapChainDesc, &d3dSwapChain, &d3dDevice, &featureLevel,
		&d3dDeviceContext);

	if (hr == E_INVALIDARG)
	{
		hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE,
			nullptr, createDeviceFlags, &featureLevels[1], _countof(featureLevels) - 1,
			D3D11_SDK_VERSION, &swapChainDesc, &d3dSwapChain, &d3dDevice, &featureLevel,
			&d3dDeviceContext);
	}

	if (FAILED(hr))
	{
		return -1;
	}



	//Next initalize the back buffer of the swap chain and associate it to a
	// render target view
	ID3D11Texture2D* backBuffer;
	hr = d3dSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(hr))
	{
		return -1;
	}

	hr = d3dDevice->CreateRenderTargetView(backBuffer, nullptr, &d3dRenderTargetView);
	if (FAILED(hr))
	{
		return -1;
	}

	SafeRelease(backBuffer);

	// Create the depth buffer for use with the depth/stencil view
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
	ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0; // No CPU access required
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.Width = clientWidth;
	depthStencilBufferDesc.Height = clientHeight;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = d3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &d3dDepthStencilBuffer);
	if (FAILED(hr))
	{
		return -1;
	}

	hr = d3dDevice->CreateDepthStencilView(d3dDepthStencilBuffer, nullptr, &d3dDepthStencilView);
	if (FAILED(hr))
	{
		return -1;
	}

	// setup depth/stencil state
	D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
	ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthStencilStateDesc.DepthEnable = TRUE;
	depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilStateDesc.StencilEnable = FALSE;

	hr = d3dDevice->CreateDepthStencilState(&depthStencilStateDesc, &d3dDepthStencilState);

	//setup rasterizer state
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthBias = 0;
	rasterizerDesc.DepthBiasClamp = 0.0f;
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.ScissorEnable = FALSE;
	rasterizerDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state object
	hr = d3dDevice->CreateRasterizerState(&rasterizerDesc, &d3dRasterizerState);
	if (FAILED(hr))
	{
		return -1;
	}

	//Initalize the viewport to occupy the entire client area
	Viewport.Width = static_cast<float>(clientWidth);
	Viewport.Height = static_cast<float>(clientHeight);
	Viewport.TopLeftX = 0.0f;
	Viewport.TopLeftY = 0.0f;
	Viewport.MinDepth = 0.0f;
	Viewport.MaxDepth = 1.0f;

	return 0;
}

void DX11Render::Render()
{
	assert(d3dDevice);
	assert(d3dDeviceContext);

	Clear(Colors::CornflowerBlue, 1.0f, 0);

	const UINT vertexStride = sizeof(VertexPosColor);
	const UINT offset = 0;

	d3dDeviceContext->IASetVertexBuffers(0, 1, &d3dVertexBuffer, &vertexStride, &offset);
	d3dDeviceContext->IASetInputLayout(d3dInputLayout);
	d3dDeviceContext->IASetIndexBuffer(d3dIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	d3dDeviceContext->VSSetShader(d3dVertexShader, nullptr, 0);
	d3dDeviceContext->VSSetConstantBuffers(0, 3, d3dConstantBuffers);

	d3dDeviceContext->RSSetState(d3dRasterizerState);
	d3dDeviceContext->RSSetViewports(1, &Viewport);

	d3dDeviceContext->PSSetShader(d3dPixelShader, nullptr, 0);

	d3dDeviceContext->OMSetRenderTargets(1, &d3dRenderTargetView, d3dDepthStencilView);
	d3dDeviceContext->OMSetDepthStencilState(d3dDepthStencilState, 1);

	d3dDeviceContext->DrawIndexed(_countof(Indicies), 0, 0);

	Present(EnableVSync);
}

void DX11Render::Present(bool vSync)
{
	if (vSync)
	{
		d3dSwapChain->Present(1, 0);
	}
	else
	{
		d3dSwapChain->Present(0, 0);
	}
}

bool DX11Render::LoadContent()
{
	assert(d3dDevice);

	//Create an initialize the vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(D3D11_BUFFER_DESC));

	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(VertexPosColor)*_countof(Vertices);
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(D3D11_SUBRESOURCE_DATA));

	resourceData.pSysMem = Vertices;

	HRESULT hr = d3dDevice->CreateBuffer(&vertexBufferDesc, &resourceData, &d3dVertexBuffer);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("vertex buffer"), TEXT("ERROR"), MB_OK);
		return false;
	}

	// Create and initialize the index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(D3D11_BUFFER_DESC));

	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.ByteWidth = sizeof(WORD)*_countof(Indicies);
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	resourceData.pSysMem = Indicies;

	hr = d3dDevice->CreateBuffer(&indexBufferDesc, &resourceData, &d3dIndexBuffer);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("inex buffer"), TEXT("ERROR"), MB_OK);
		return false;
	}

	// Create constant buffers for the variable defined in the vertex shader
	D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(D3D11_BUFFER_DESC));

	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.ByteWidth = sizeof(XMMATRIX);
	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

	hr = d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &d3dConstantBuffers[CB_Application]);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("CB_Application"), TEXT("ERROR"), MB_OK);
		return false;
	}
	hr = d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &d3dConstantBuffers[CB_Frame]);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("CB_Frame"), TEXT("ERROR"), MB_OK);
		return false;
	}
	hr = d3dDevice->CreateBuffer(&constantBufferDesc, nullptr, &d3dConstantBuffers[CB_Object]);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("CB_Object"), TEXT("ERROR"), MB_OK);
		return false;
	}

	//laod the compiled vertex shader
	ID3DBlob* vertexShaderBlob;
#if _DEBUG
	LPCWSTR compiledVertexShaderObject = L"SimpleVertexShader.cso";
#else
	LPCWSTR compiledVertexShaderObject = L"SimpleVertexShader.cso";
#endif

	hr = D3DReadFileToBlob(compiledVertexShaderObject, &vertexShaderBlob);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("D3DReadFileToBlob vertex"), TEXT("ERROR"), MB_OK);
		return false;
	}

	hr = d3dDevice->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &d3dVertexShader);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("CreateVertexShader"), TEXT("ERROR"), MB_OK);
		return false;
	}

	//Create the input laout for the vertex shader.	
	D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor,Position),D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR", 0 , DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VertexPosColor, Color), D3D11_INPUT_PER_VERTEX_DATA,0 }
	};

	hr = d3dDevice->CreateInputLayout(vertexLayoutDesc, _countof(vertexLayoutDesc), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &d3dInputLayout);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("CreateInputLayout :P"), TEXT("ERROR"), MB_OK);
		return false;
	}

	SafeRelease(vertexShaderBlob);

	//Load the compiled pixel shader
	ID3DBlob* pixelShaderBlob;
#if _DEBUG
	LPCWSTR compiledPixelShaderObject = L"SimplePixelShader.cso";
#else
	LPCWSTR compiledPixelShaderObject = L"SimplePixelShader.cso";
#endif

	hr = D3DReadFileToBlob(compiledPixelShaderObject, &pixelShaderBlob);
	if (FAILED(hr))
	{
		//	return false;
		MessageBox(nullptr, TEXT("D3DReadFileToBlob pixel"), TEXT("ERROR"), MB_OK);
		return false;
	}

	hr = d3dDevice->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &d3dPixelShader);
	if (FAILED(hr))
	{
		//return false;
		MessageBox(nullptr, TEXT("CreatePixelShader"), TEXT("ERROR"), MB_OK);
		return false;
	}

	SafeRelease(pixelShaderBlob);

	//Setup the projection matrix.
	RECT clientRect;
	GetClientRect(Window::Instance()->GetWindowHandle(), &clientRect);

	//compute the exact client dimensions.
	//the is required for a correct projection matrix.
	float clientWidth = static_cast<float>(clientRect.right - clientRect.left);
	float clientHeight = static_cast<float>(clientRect.bottom - clientRect.top);

	ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), clientWidth / clientHeight, 0.1f, 100.0f);

	d3dDeviceContext->UpdateSubresource(d3dConstantBuffers[CB_Application], 0, nullptr, &ProjectionMatrix, 0, 0);

	return true;
}

//Clear the color and depth buffers.
void DX11Render::Clear(const FLOAT clearColor[4], FLOAT clearDepth, UINT8 clearStencil)
{
	d3dDeviceContext->ClearRenderTargetView(d3dRenderTargetView, clearColor);
	d3dDeviceContext->ClearDepthStencilView(d3dDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepth, clearStencil);
}

void DX11Render::Update(float deltaTime)
{
	XMVECTOR eyePosition = XMVectorSet(0, 0, -10, 1);
	XMVECTOR focusPoint = XMVectorSet(0, 0, 0, 1);
	XMVECTOR upDirection = XMVectorSet(0, 1, 0, 0);
	ViewMatrix = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
	d3dDeviceContext->UpdateSubresource(d3dConstantBuffers[CB_Frame], 0, nullptr, &ViewMatrix, 0, 0);


	static float angle = 0.0f;
	angle += 90.0f*deltaTime;
	XMVECTOR rotationAxis = XMVectorSet(0, 1, 1, 0);

	WorldMatrix = XMMatrixRotationAxis(rotationAxis, XMConvertToRadians(angle));
	d3dDeviceContext->UpdateSubresource(d3dConstantBuffers[CB_Object], 0, nullptr, &WorldMatrix, 0, 0);
}



void DX11Render::UnloadContent()
{
	SafeRelease(d3dConstantBuffers[CB_Application]);
	SafeRelease(d3dConstantBuffers[CB_Frame]);
	SafeRelease(d3dConstantBuffers[CB_Object]);
	SafeRelease(d3dIndexBuffer);
	SafeRelease(d3dVertexBuffer);
	SafeRelease(d3dInputLayout);
	SafeRelease(d3dVertexShader);
	SafeRelease(d3dPixelShader);
}

void DX11Render::Cleanup()
{
	UnloadContent();

	SafeRelease(d3dDepthStencilView);
	SafeRelease(d3dRenderTargetView);
	SafeRelease(d3dDepthStencilBuffer);
	SafeRelease(d3dDepthStencilState);
	SafeRelease(d3dRasterizerState);
	SafeRelease(d3dSwapChain);
	SafeRelease(d3dDeviceContext);
	SafeRelease(d3dDevice);
}