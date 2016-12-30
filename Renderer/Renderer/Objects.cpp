#include "DirectXTemplatePCH.h"
#include "Object.h"


Cube::Cube() {
}

int Cube::getCountOfVerticies() {
	return _countof(Vertices);
}

int Cube::getCountOfIndicies() {
	return _countof(Indicies);
}

VertexPosColor* Cube::getVerticies() {
	return &Vertices[0];
}

WORD* Cube::getIndicies() {
	return &Indicies[0];
}