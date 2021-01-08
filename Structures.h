#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;

	bool operator<(const SimpleVertex other) const
	{
		return memcmp((void*)this, (void*)& other, sizeof(SimpleVertex)) > 0;
	};
};

struct MeshData
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

struct ConstantBuffer
{
	float gTime;

	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	//For Lighting - (Diffuse)
	float pad; //Pad Value from Float3 to Float4
	XMFLOAT3 LightVecW;
	XMFLOAT4 diffuseMtrl;
	XMFLOAT4 diffuseLight;

	//For Lighting - (Ambient)
	XMFLOAT4 ambientMtrl;
	XMFLOAT4 ambientLight;

	//For Lighting - (Specular)
	XMFLOAT4 SpecularMtrl;
	XMFLOAT4 SpecularLight;
	float SpecularPower;
	XMFLOAT3 EyePosW;

	
};

struct SCamera
{
	XMVECTOR _eye;
	XMVECTOR _at;
	XMVECTOR _up;
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;


	FLOAT _windowWidth;
	FLOAT _windowHeight;
	FLOAT _nearDepth;
	FLOAT _farDepth;


};
