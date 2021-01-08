#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "DDSTextureLoader.h"
#include "OBJLoader.h"
#include "Structures.h"

class Camera
{
private:



public:
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, bool lookTo);
	~Camera();
	void SetLookAtMatrix();
	void SetLookToMatrix();
	void Update(bool lookTo);
	void MoveFirstPerson(XMFLOAT3 position, bool lookTo);
	void MoveThirdPerson(XMFLOAT3 position, XMFLOAT3 at, bool lookTo);
	SCamera camera;
};


