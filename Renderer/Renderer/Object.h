#pragma once
#include "DirectXTemplatePCH.h"
#include "DX11Render.h"



class Cube
{
public:
	Cube();
	int getCountOfVerticies();
	int getCountOfIndicies();
	VertexPosColor *getVerticies();
	WORD *getIndicies();

private:

	VertexPosColor Vertices[8] = {
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f,  0.0f,  0.0f) }, //0
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT3(0.0f,  1.0f,  0.0f) }, //1
		{ XMFLOAT3(1.0f,  1.0f, -1.0f), XMFLOAT3(1.0f,  1.0f,  0.0f) }, //2
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f,  0.0f,  0.0f) }, //4
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT3(0.0f,  0.0f,  1.0f) }, //5
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT3(0.0f,  1.0f,  1.0f) }, //6
		{ XMFLOAT3(1.0f,  1.0f,  1.0f), XMFLOAT3(1.0f,  1.0f,  1.0f) }, //7
		{ XMFLOAT3(1.0f, -1.0f,  1.0f), XMFLOAT3(1.0f,  0.0f,  1.0f) }, //8
	};

	WORD Indicies[36] = {
		0, 1, 2, 0, 2, 3,
		4, 6, 5, 4, 7, 6,
		4, 5, 1, 4, 1, 0,
		3, 2, 6, 3, 6, 7,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7
	};
};