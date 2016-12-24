#pragma once
// System includes
#include<Windows.h>

// DirectX includes
#include<d3d11.h>
#include<d3dcompiler.h>
#include<DirectXMath.h>
#include<DirectXColors.h>

// STL includes
#include<iostream>
#include<string>

// Link Library dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")

// Safely release a COM object
#include<algorithm>//added in to fix an error with the std::min in the run function of main
template<typename T>
inline void SafeRelease(T& ptr)
{
	if (ptr != NULL)
	{
		ptr->Release();
		ptr = NULL;
	}
}

//https://github.com/parcPrograms/Renderer