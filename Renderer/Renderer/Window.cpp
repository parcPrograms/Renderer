#include "DirectXTemplatePCH.h"
#include "Window.h"

Window* Window::instance = 0;

Window* Window::Instance() {
	if (instance==NULL) {
		instance = new Window();
	}
	return instance;
}

HWND Window::GetWindowHandle() {
	return windowHandle;
}

Window::Window() {
	windowClassName = "DirectXWindowClass";
	windowName = "DirectX Temlate";
	windowHandle = 0;
}

/**
* Initialize the Application window.
*/
int Window::InitApplication(HINSTANCE hInstance, int cmdShow)
{
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = &WndProc;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = nullptr;
	wndClass.lpszClassName = windowClassName;

	if (!RegisterClassEx(&wndClass))
	{
		return -1;
	}


	RECT windowRect = { 0,0,windowWidth, windowHeight };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	windowHandle = CreateWindowA(windowClassName, windowName,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance, nullptr);

	if (!windowHandle)
	{
		return -1;
	}

	ShowWindow(windowHandle, cmdShow);
	UpdateWindow(windowHandle);

	return 0;
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