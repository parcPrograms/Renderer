#include "DirectXTemplatePCH.h"
using namespace DirectX;

const LONG g_WindowWidth = 1280;
const LONG g_WindowHeight = 720;
LPCSTR g_WindowClassName = "DirectXWindowClass";
LPCSTR g_WindowName = "DirectX Temlate";
HWND g_WindowHandle = 0;

const BOOL g_EnableVSync = true;

// Direct3d device and swap chain.
ID3D11Device* g_d3dDevice = nullptr;
ID3D11DeviceContext* g_d3dDeviceContext = nullptr;
IDXGISwapChain* g_d3dSwapChain = nullptr;

// Render target view for the back buffer of the swap chain.
ID3D11RenderTargetView* g_d3dRenderTargetView = nullptr;
// Depth/stencil view for use as a depth buffer.
ID3D11DepthStencilView* g_d3dDepthStencilView = nullptr;
// A texture to assiciate to the depth stencil view.
ID3D11Texture2D* g_d3dDepthStencilBuffer = nullptr;

// Define the functionality of the depth/stencil stages
ID3D11DepthStencilState* g_d3dDepthStencilState = nullptr;
// Define the functionality of the rasterizer stage
ID3D11RasterizerState* g_d3dRasterizerState = nullptr;
D3D11_VIEWPORT g_Viewport = { 0 };

// Vertex buffer data
ID3D11InputLayout* g_d3dInputLayout = nullptr;
ID3D11Buffer* g_d3dVertexBuffer = nullptr;
ID3D11Buffer* g_d3dIndexBuffer = nullptr;

// Shader data
ID3D11VertexShader* g_d3dVertexShader = nullptr;
ID3D11PixelShader* g_d3dPixelShader = nullptr;

// Shader resources
enum  ConstanBuffer
{
	CB_Application,
	CB_Frame,
	CB_Object,
	NumConstantBuffers
};

ID3D11Buffer* g_d3dConstantBuffers[NumConstantBuffers];

// Demo parameters
XMMATRIX g_WorldMatrix;
XMMATRIX g_ViewMatrix;
XMMATRIX g_ProjectionMatrix;

// Vertex data for a colored cube.
struct VertexPosColor
{
	XMFLOAT3 Postion;
	XMFLOAT3 Color;
};

VertexPosColor g_Vertices[8] =
{
	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f,  0.0f,  0.0f)}, //0
	{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f)}, //1
	{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f,  1.0f,  0.0f)}, //2
	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f,  0.0f,  0.0f)}, //4
	{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f,  0.0f,  1.0f)}, //5
	{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f,  1.0f,  1.0f)}, //6
	{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f,  1.0f,  1.0f)}, //7
	{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f,  0.0f,  1.0f)}, //8
};

WORD g_Indicies[36] =
{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 2, 7
};



// Forward declarations.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

template< class ShaderClass >
ShaderClass* LoadShader(const std::wstring& fileName, const std::string& entryPoint, const std::string& profile);

bool LoadContent();
void UnloadContent();

void Update(float deltaTime);
void Render();
void Cleanup();

/**
 * Initialize the Application window.
 */
int InitApplication(HINSTANCE hInstance, int cmdShow)
{
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = &WndProc;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = g_WindowClassName;

	if (!RegisterClassEx(&wndClass))
	{
		return -1;
	}


	RECT windowRect = { 0,0,g_WindowWidth, g_WindowHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	g_WindowHandle=CreateWindowA(g_WindowClassName, g_WindowName,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance, nullptr);
	
	if (!g_WindowHandle)
	{
		return -1;
	}

	ShowWindow(g_WindowHandle, cmdShow);
	UpdateWindow(g_WindowHandle);

	return 0;
}



/**
* INitalize the DirectX device and swap chain.
*/
int InitDirectX(HINSTANCE hInstance, BOOL vSync)
{
	// A window handle must have been created already.
	assert(g_WindowHandle != 0);

	RECT clientRect;
	GetClientRect(g_WindowHandle, &clientRect);

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
	swapChainDesc.OutputWindow = g_WindowHandle;
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
		D3D11_SDK_VERSION, &swapChainDesc, &g_d3dSwapChain, &g_d3dDevice, &featureLevel,
		&g_d3dDeviceContext);

	if(hr == E_INVALIDARG)
	{
		hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE,
			nullptr, createDeviceFlags, &featureLevels[1], _countof(featureLevels) - 1,
			D3D11_SDK_VERSION, &swapChainDesc, &g_d3dSwapChain, &g_d3dDevice, &featureLevel,
			&g_d3dDeviceContext);
	}

	if (FAILED(hr))
	{
		return -1;
	}
}


/**
* The main application loop.
*/
int Run()
{
	MSG msg = { 0 };

	static DWORD previousTime = timeGetTime();
	static const float targetFramrate = 30.0f;
	static const float maxTimeStep = 1.0f / targetFramrate;

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			DWORD currentTime = timeGetTime();
			float deltaTime = (currentTime - previousTime) / 1000.0f;
			previousTime = currentTime;

			// Cap the delta time to the max time step ( useful if your
			// debugging and you don't want the deltaTime value to explode.
			deltaTime = std::min<float>(deltaTime, maxTimeStep);

			//Update(deltaTime);
			//Render();
		}
	}

	return static_cast<int>(msg.wParam);

}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);

	// Check for DirectX Math library support.
	if (!XMVerifyCPUSupport())
	{
		MessageBox(nullptr, TEXT("Failed to verify DirectX Mazt library support."), TEXT("Error"), MB_OK);
		return -1;
	}

	if (InitApplication(hInstance, cmdShow) != 0)
	{
		MessageBox(nullptr, TEXT("Failed to create application window."), TEXT("Error"), MB_OK);
		return -1;
	}

	int returnCode = Run();

	return returnCode;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT paintStruct;
	HDC hDC;

	switch (message)
	{
	case WM_PAINT:
	{
		hDC = BeginPaint(hwnd, &paintStruct);
		EndPaint(hwnd, &paintStruct);
	}
	break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}
	break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}