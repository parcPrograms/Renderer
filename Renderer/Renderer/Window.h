#pragma once

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

class Window
{
public:
	static Window* Instance();
	HWND GetWindowHandle();

	int InitApplication(HINSTANCE hInstance, int cmdShow);
	
private:
	Window();
	static Window* instance;


	// Forward declarations.
	


	const LONG windowWidth = 1280;
	const LONG windowHeight = 720;
	LPCSTR windowClassName;
	LPCSTR windowName;
	HWND windowHandle;
};