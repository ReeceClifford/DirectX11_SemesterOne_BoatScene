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
#include "Camera.h"
#include <stdlib.h>     /* srand, rand */

using namespace DirectX;


class Application
{

private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pConstantBuffer;

	//For Depth and Stencil Buffer
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D*		_depthStencilBuffer;
	XMFLOAT4X4              _world, _world2, _world3;
	XMFLOAT4X4				_rocks[28];
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;

	//Render States
	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _solidFrame; 
	ID3D11RasterizerState* _currentState;

	ConstantBuffer cb;

	//For Lighting - (Diffuse)
	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	//For Lighting - (Ambient)
	XMFLOAT4 ambientMaterial;
	XMFLOAT4 ambientLight;

	//For Lighting - (Specular)
	XMFLOAT4 specularMaterial;
	XMFLOAT4 specularLight;
	float specularPower;
	XMFLOAT3 eyePosW;

	//Added for Texutring Process
	ID3D11ShaderResourceView* _pTextureRV;
	ID3D11ShaderResourceView* _pTextureRVWater;
	ID3D11ShaderResourceView* _pTextureRVRock;
	ID3D11ShaderResourceView* _pTextureRVSky;
	ID3D11SamplerState* _pSamplerLinear;

	//Added for OBJLoader Process
	MeshData objMeshDataBoat;
	MeshData objMeshDataWater;
	MeshData objMeshDataRock;
	MeshData objMeshDataSky;

	Camera* freeMoveCamera;
	Camera* firstPersonCamera;
	Camera* staticBirdsEyeCamera;
	Camera* thirdPersonCamera;
	Camera* staticPerspectiveCamera;
	float cameraActive;
	XMFLOAT3 eye;
	XMFLOAT3 at;
	XMFLOAT3 up;

	//Boat Values
	
	XMMATRIX playerBoat;
	XMVECTOR boatFacingDirection;
	XMVECTOR boatUp;
	XMVECTOR boatScale;
	XMFLOAT4X4 fCurrentBoatPosition;
	XMFLOAT3 boatPosition;
	XMFLOAT3 firstPersonBoatCameraPosition;
	XMFLOAT3 thirdPersonBoatCameraPosition;

	//Created for Water
	ID3D11VertexShader* _pVertexShaderWater;
	ID3D11PixelShader* _pPixelShaderWater;
	ID3D11InputLayout* _pVertexLayoutWater;

	
private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};
