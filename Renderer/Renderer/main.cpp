#include "DirectXTemplatePCH.h"
#include "DX11Render.h"
#include "Window.h"


void Update(float deltaTime);
void Cleanup();
int Run();


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPWSTR cmdLine, int cmdShow)
{
	UNREFERENCED_PARAMETER(prevInstance);
	UNREFERENCED_PARAMETER(cmdLine);

	// Check for DirectX Math library support.
	if (!XMVerifyCPUSupport()){
		MessageBox(nullptr, TEXT("Failed to verify DirectX Mazt library support."), TEXT("Error"), MB_OK);
		return -1;
	}
	if (Window::Instance()->InitApplication(hInstance, cmdShow) != 0){
		MessageBox(nullptr, TEXT("Failed to create application window."), TEXT("Error"), MB_OK);
		return -1;
	}
	if (DX11Render::Instance()->InitDirectX(hInstance, DX11Render::Instance()->EnableVSync) != 0){
		MessageBox(nullptr, TEXT("Failed to creat DirectX device and swap chain."), TEXT("Error"), MB_OK);
		return -1;
	}
	if (!DX11Render::Instance()->LoadContent()){
		MessageBox(nullptr, TEXT("Failed to load content."), TEXT("ERROR"), MB_OK);
		return -1;
	}

	int returnCode = Run();

	Cleanup();

	return returnCode;
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

			Update(deltaTime);
			DX11Render::Instance()->Render();
		}
	}
	return static_cast<int>(msg.wParam);
}

void Update(float deltaTime)
{
	DX11Render::Instance()->Update(deltaTime);
//call the dx updat
}

void Cleanup()
{
	DX11Render::Instance()->Cleanup();
//call dx clean up
}


//TODO:  use this template to replace the sizeof stuf used in the demo  
//template<typename T, std::size_t N = sizeof(T)>void ZeroMemory2(T* const t){ ::ZeroMemory(t, N);} 