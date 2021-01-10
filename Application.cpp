#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pConstantBuffer = nullptr;

	//Added for Texturing
	_pTextureRV = nullptr;
	_pSamplerLinear = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
		return E_FAIL;
	}

	RECT rc;
	GetClientRect(_hWnd, &rc);
	_WindowWidth = rc.right - rc.left;
	_WindowHeight = rc.bottom - rc.top;

	if (FAILED(InitDevice()))
	{
		Cleanup();

		return E_FAIL;
	}

	//
	// Initialize the world matrix
	//
	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	XMStoreFloat4x4(&_world, XMMatrixScaling(0.25f, 0.25f, 0.25f));


	//
	//Initialise Boat Specifc Values
	//

	boatFacingDirection = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	boatUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	//
	//Camera Settings
	//

	eye = XMFLOAT3(0.0f, 5.0f, 25.0f);
	at = XMFLOAT3(0.0f, 0.0f, -1.0f);
	up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	freeMoveCamera = new Camera(eye, at, up, _WindowWidth, _WindowHeight, 0.01f, 150.0f, true); // Dynamic  Free Moving Camera

	eye = XMFLOAT3(0.0f, 0.0f, 0.0f);
	at = XMFLOAT3(0.0f, 0.0f, -1.0f); 
	up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	firstPersonCamera = new Camera(eye, at, up, _WindowWidth, _WindowHeight, 0.01f, 150.0f, false); //First Person Camera

	eye = XMFLOAT3(0.0f, 65.0f, 0.0f);
	at = XMFLOAT3(0.0f, 0.0f, 0.0f);
	up = XMFLOAT3(1.0f, 0.0f, 0.0f);

	staticBirdsEyeCamera = new Camera(eye, at, up, _WindowWidth, _WindowHeight, 0.01f, 250.0f, true);  //Birds Eye View Camera

	eye = XMFLOAT3(0.0f, 20.0f, -15.0f);
	at = XMFLOAT3(0.0f, 0.0f, 0.0f);
	up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	thirdPersonCamera = new Camera(eye, at, up, _WindowWidth, _WindowHeight, 0.01f, 150.0f, true); 

	
	eye = XMFLOAT3(10.0f, 50.0f, 15.0f);
	at = XMFLOAT3(0.0f, 0.0f, 0.0f);
	up = XMFLOAT3(0.0f, 1.0f, 0.0f);

	staticPerspectiveCamera = new Camera(eye, at, up, _WindowWidth, _WindowHeight, 0.01f, 250.0f, true); 

	_view = freeMoveCamera->camera._view;
	_projection = freeMoveCamera->camera._projection;
	cameraActive = 1;
	
	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

	//
	// Compile and Create Vertex Shaders
	//

	// Standard VS
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Water VS
	ID3DBlob* pVSBlobWater = nullptr;
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "VSWATER", "vs_4_0", &pVSBlobWater);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	hr = _pd3dDevice->CreateVertexShader(pVSBlobWater->GetBufferPointer(), pVSBlobWater->GetBufferSize(), nullptr, &_pVertexShaderWater);

	if (FAILED(hr))
	{
		pVSBlobWater->Release();
		return hr;
	}


	//
	// Compile and Create the Pixel Shader
	//

	// Standard Pixel Shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

	if (FAILED(hr))
		return hr;

	// Water Pixel Shader
	ID3DBlob* pPSBlobWater = nullptr;
	hr = CompileShaderFromFile(L"DX11 Framework.fx", "PSWATER", "ps_4_0", &pPSBlobWater);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	hr = _pd3dDevice->CreatePixelShader(pPSBlobWater->GetBufferPointer(), pPSBlobWater->GetBufferSize(), nullptr, &_pPixelShaderWater);
	pPSBlobWater->Release();

	if (FAILED(hr))
		return hr;


	//
	// Define the input 
	//

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	//
	// Create the input layout
	//

	// Standard Input Layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
		return hr;

	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlobWater->GetBufferPointer(),
		pVSBlobWater->GetBufferSize(), &_pVertexLayoutWater);
	pVSBlobWater->Release();

	if (FAILED(hr))
		return hr;


	// Set the input layout
	_pImmediateContext->IASetInputLayout(_pVertexLayout);
	_pImmediateContext->IASetInputLayout(_pVertexLayoutWater);
	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

	//
	// Create Vertex Buffer use this when not using an OBJ Loader for files
	//

	
	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

	//
	// Create index buffer use this when not using an OBJ loader for files
	//

	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	//
	// Register class
	//

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	//
	// Create Program Window
	//

	_hInst = hInstance;
	RECT rc = { 0, 0, 1920, 1080 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct X 11 - Marine Ship Scene", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!_hWnd)
		return E_FAIL;

	ShowWindow(_hWnd, nCmdShow);


	return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

		if (pErrorBlob) pErrorBlob->Release();

		return hr;
	}

	if (pErrorBlob) pErrorBlob->Release();



	return S_OK;
}

HRESULT Application::InitDevice()
{
	HRESULT hr = S_OK;



	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _WindowWidth;
	sd.BufferDesc.Height = _WindowHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	//
	// Define Depth / Stencil Buffer
	//

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

	//
	// Create a render target view
	//

	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	if (FAILED(hr))
		return hr;

	hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
	pBackBuffer->Release();

	if (FAILED(hr))
		return hr;

	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

	//
	// Setup the viewport
	//

	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)_WindowWidth;
	vp.Height = (FLOAT)_WindowHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();

	//
	// Set vertex buffer for hard coded values
	//

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;


	InitIndexBuffer();

	//
	// Set index buffer for hard codes values
	//


	//
	// Set primitive topology
	//
	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	//
	// Create the constant buffer
	//

	// Standard Constant Buffer

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	if (FAILED(hr))
		return hr;


	//
	// Load in OBJ Model
	//

	objMeshDataBoat = OBJLoader::Load("mainPlayerBoat.obj", _pd3dDevice, false);
	objMeshDataWater = OBJLoader::Load("water.obj", _pd3dDevice, false);
	objMeshDataRock = OBJLoader::Load("rockBorder.obj", _pd3dDevice, false);
	objMeshDataSky = OBJLoader::Load("skyboxSphere.obj", _pd3dDevice, false);

	//
	// Create the sample state - Texturing
	//

	CreateDDSTextureFromFile(_pd3dDevice, L"mainPlayerBoatTex.dds", nullptr, &_pTextureRV);
	CreateDDSTextureFromFile(_pd3dDevice, L"oceanTex.dds", nullptr, &_pTextureRVWater);
	CreateDDSTextureFromFile(_pd3dDevice, L"rock.dds", nullptr, &_pTextureRVRock);
	CreateDDSTextureFromFile(_pd3dDevice, L"sky.dds", nullptr, &_pTextureRVSky);

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//
	// Solid Frame - Wire Frame
	//

	D3D11_RASTERIZER_DESC wfdesc;
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	wfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

	D3D11_RASTERIZER_DESC sfdesc;
	ZeroMemory(&sfdesc, sizeof(D3D11_RASTERIZER_DESC));
	sfdesc.FillMode = D3D11_FILL_SOLID;
	sfdesc.CullMode = D3D11_CULL_NONE;
	hr = _pd3dDevice->CreateRasterizerState(&sfdesc, &_solidFrame);

	// 
	//Lighting Values
	//

	// Light direction from surface (XYZ)
	lightDirection = XMFLOAT3(0.25f, 0.5f, -1.0f);
	// Diffuse material properties (RGBA)
	diffuseMaterial = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	// Diffuse light colour (RGBA)
	diffuseLight = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	// Ambient Material properties
	ambientMaterial = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	// Ambient Material Properties
	ambientLight = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);

	//Specular
	specularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	specularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	specularPower = 10.0f;
	eyePosW = XMFLOAT3(0.0f, 0.0f, -3.0f);

	return S_OK;
}

void Application::Cleanup()
{
	if (_pImmediateContext) _pImmediateContext->ClearState();
	if (_pConstantBuffer) _pConstantBuffer->Release();
	if (_pVertexLayout) _pVertexLayout->Release();
	if (_pVertexShader) _pVertexShader->Release();
	if (_pPixelShader) _pPixelShader->Release();
	if (_pRenderTargetView) _pRenderTargetView->Release();
	if (_pSwapChain) _pSwapChain->Release();
	if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
	if (_wireFrame) _wireFrame->Release();
	if (_solidFrame) _solidFrame->Release();
}

void Application::Update()
{
	//
	// Update our time
	//
	static float t = 0.0f;

	if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();

		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;

		t = (dwTimeCur - dwTimeStart) / 1000.0f;

		
		cb.gTime = t;
	}
	//
	// User Inputted Controls
	//

	XMVECTOR freeMoveCameraRight = XMVector3Cross(freeMoveCamera->camera._at, freeMoveCamera->camera._up);
	XMVECTOR playerBoatRight = XMVector3Cross(boatFacingDirection, boatUp);
	playerBoat = XMLoadFloat4x4(&_world);

	//
	// Change Camera Being Used
	//

	if (GetKeyState(VK_NUMPAD1) & 0x8000) //Camera One (LookTo Free Move))
	{
		cameraActive = 1;
		_view = freeMoveCamera->camera._view;
		_projection = freeMoveCamera->camera._projection;
	}
	else if (GetKeyState(VK_NUMPAD2) & 0x8000) //Camera Two (LookTo 1st Person)
	{
		cameraActive = 2;

		XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
		firstPersonBoatCameraPosition.x = fCurrentBoatPosition._41;
		firstPersonBoatCameraPosition.y = fCurrentBoatPosition._42 + 2.0f;
		firstPersonBoatCameraPosition.z = fCurrentBoatPosition._43;

		firstPersonBoatCameraPosition = XMFLOAT3(firstPersonBoatCameraPosition.x, firstPersonBoatCameraPosition.y, firstPersonBoatCameraPosition.z);
		
		if (cameraActive == 2)
		{
			firstPersonCamera->MoveFirstPerson(firstPersonBoatCameraPosition, false);
			firstPersonCamera->camera._at = boatFacingDirection;
			_view = firstPersonCamera->camera._view;
			_projection = firstPersonCamera->camera._projection;
		}
	}
	else if (GetKeyState(VK_NUMPAD3) & 0x8000) // Camera Three (LookAt BirdsEye)
	{
		cameraActive = 3;
		_view = staticBirdsEyeCamera->camera._view;
		_projection = staticBirdsEyeCamera->camera._projection;
	}
	else if (GetKeyState(VK_NUMPAD4) & 0x8000) // Camera Four (LookAt 3rd Person)
	{
		cameraActive = 4;

		XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
		boatPosition.x = fCurrentBoatPosition._41;
		boatPosition.y = fCurrentBoatPosition._42;
		boatPosition.z = fCurrentBoatPosition._43;

		boatPosition = XMFLOAT3(boatPosition.x, boatPosition.y, boatPosition.z);

		thirdPersonBoatCameraPosition = XMFLOAT3(boatPosition.x, boatPosition.y + 20.0f, boatPosition.z - 15.0f);

		thirdPersonCamera->MoveThirdPerson(thirdPersonBoatCameraPosition, boatPosition, true);

		_view = thirdPersonCamera->camera._view;
		_projection = thirdPersonCamera->camera._projection;
	}
	else if (GetKeyState(VK_NUMPAD5) & 0x8000) // Camera 5 Static Viewpoint Perspective
	{
		cameraActive = 5;

		_view = staticPerspectiveCamera->camera._view;
		_projection = staticPerspectiveCamera->camera._projection;
	}
	//
	// Camera One Controls
	//

	if (GetAsyncKeyState(0x57))// W - Move Camera Forwards
	{
		if (cameraActive == 1)
		{
			freeMoveCamera->camera._eye = freeMoveCamera->camera._eye + (freeMoveCamera->camera._at * 0.01f);
			freeMoveCamera->Update(false);
			_view = freeMoveCamera->camera._view;
		}

	}
	else if (GetAsyncKeyState(0x41))// A - Move Camera Left
	{
		if (cameraActive == 1)
		{
			freeMoveCamera->camera._eye = freeMoveCamera->camera._eye + (freeMoveCameraRight * 0.01f);
			freeMoveCamera->Update(false);
			_view = freeMoveCamera->camera._view;
		}

	}
	else if (GetAsyncKeyState(0x44))// D - Move Camera Right
	{
		if (cameraActive == 1)
		{
			freeMoveCamera->camera._eye = freeMoveCamera->camera._eye - (freeMoveCameraRight * 0.01f);
			freeMoveCamera->Update(false);
			_view = freeMoveCamera->camera._view;
		}
	}
	else if (GetAsyncKeyState(0x53))// S - Move Camera Backwards
	{
		if (cameraActive == 1)
		{
			freeMoveCamera->camera._eye = freeMoveCamera->camera._eye - (freeMoveCamera->camera._at * 0.01f);
			freeMoveCamera->Update(false);
			_view = freeMoveCamera->camera._view;
		}
	}

	//
	//Boat Controls
	//

	if (GetAsyncKeyState(VK_RIGHT)) // Right Key - Rotate Boat Clockwise
	{
		playerBoat = XMMatrixRotationRollPitchYaw(-0.00f, 0.0003f, 0.0f) * playerBoat;
		boatFacingDirection = (boatFacingDirection - (playerBoatRight * 0.0003f));
		
		if (cameraActive == 2)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			firstPersonBoatCameraPosition.x = fCurrentBoatPosition._41;
			firstPersonBoatCameraPosition.y = fCurrentBoatPosition._42 + 2.0f;
			firstPersonBoatCameraPosition.z = fCurrentBoatPosition._43;

			firstPersonBoatCameraPosition = XMFLOAT3(firstPersonBoatCameraPosition.x, firstPersonBoatCameraPosition.y, firstPersonBoatCameraPosition.z);

			firstPersonCamera->MoveFirstPerson(firstPersonBoatCameraPosition, false);
			firstPersonCamera->camera._at = boatFacingDirection;
			_view = firstPersonCamera->camera._view;
		}	
		else if (cameraActive == 4)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			boatPosition.x = fCurrentBoatPosition._41;
			boatPosition.y = fCurrentBoatPosition._42;
			boatPosition.z = fCurrentBoatPosition._43;

			boatPosition = XMFLOAT3(boatPosition.x, boatPosition.y, boatPosition.z);

			thirdPersonBoatCameraPosition = XMFLOAT3(boatPosition.x, boatPosition.y + 20.0f, boatPosition.z - 15.0f);

			thirdPersonCamera->MoveThirdPerson(thirdPersonBoatCameraPosition, boatPosition, true);
			_view = thirdPersonCamera->camera._view;
		}
	}
	else if (GetAsyncKeyState(VK_LEFT)) //Left Key - Rotate Boat Anti Clockwise
	{
		playerBoat = XMMatrixRotationRollPitchYaw(-0.00f, -0.0003f, 0.0f) * playerBoat;
		boatFacingDirection = (boatFacingDirection + (playerBoatRight * 0.0003f));

		if (cameraActive == 2)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			firstPersonBoatCameraPosition.x = fCurrentBoatPosition._41;
			firstPersonBoatCameraPosition.y = fCurrentBoatPosition._42;
			firstPersonBoatCameraPosition.z = fCurrentBoatPosition._43;

			firstPersonBoatCameraPosition = XMFLOAT3(firstPersonBoatCameraPosition.x, firstPersonBoatCameraPosition.y + 2.0f, firstPersonBoatCameraPosition.z);

			firstPersonCamera->MoveFirstPerson(firstPersonBoatCameraPosition, false);
			firstPersonCamera->camera._at = boatFacingDirection;
			_view = firstPersonCamera->camera._view;
		}
		else if (cameraActive == 4)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			boatPosition.x = fCurrentBoatPosition._41;
			boatPosition.y = fCurrentBoatPosition._42;
			boatPosition.z = fCurrentBoatPosition._43;

			boatPosition = XMFLOAT3(boatPosition.x, boatPosition.y, boatPosition.z);

			thirdPersonBoatCameraPosition = XMFLOAT3(boatPosition.x, boatPosition.y + 20.0f, boatPosition.z - 15.0f);

			thirdPersonCamera->MoveThirdPerson(thirdPersonBoatCameraPosition, boatPosition, true);
			_view = thirdPersonCamera->camera._view;
		}
	}

	if (GetAsyncKeyState(VK_UP)) // Forward Key - Move Boat Forwards
	{
		playerBoat = playerBoat * XMMatrixTranslationFromVector(boatFacingDirection / 75);

		if (cameraActive == 2)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			firstPersonBoatCameraPosition.x = fCurrentBoatPosition._41;
			firstPersonBoatCameraPosition.y = fCurrentBoatPosition._42;
			firstPersonBoatCameraPosition.z = fCurrentBoatPosition._43;

			firstPersonBoatCameraPosition = XMFLOAT3(firstPersonBoatCameraPosition.x, firstPersonBoatCameraPosition.y + 2.0f, firstPersonBoatCameraPosition.z);

			firstPersonCamera->MoveFirstPerson(firstPersonBoatCameraPosition, false);
			firstPersonCamera->camera._at = boatFacingDirection;
			_view = firstPersonCamera->camera._view;
		}
		else if (cameraActive == 4)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			boatPosition.x = fCurrentBoatPosition._41;
			boatPosition.y = fCurrentBoatPosition._42;
			boatPosition.z = fCurrentBoatPosition._43;

			boatPosition = XMFLOAT3(boatPosition.x, boatPosition.y, boatPosition.z);

			thirdPersonBoatCameraPosition = XMFLOAT3(boatPosition.x, boatPosition.y + 20.0f, boatPosition.z - 15.0f);

			thirdPersonCamera->MoveThirdPerson(thirdPersonBoatCameraPosition, boatPosition, true);
			_view = thirdPersonCamera->camera._view;
		}
	}
	else if (GetAsyncKeyState(0x54))
	{
		playerBoat = playerBoat * XMMatrixTranslationFromVector(boatFacingDirection / 25);

		if (cameraActive == 2)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			firstPersonBoatCameraPosition.x = fCurrentBoatPosition._41;
			firstPersonBoatCameraPosition.y = fCurrentBoatPosition._42;
			firstPersonBoatCameraPosition.z = fCurrentBoatPosition._43;

			firstPersonBoatCameraPosition = XMFLOAT3(firstPersonBoatCameraPosition.x, firstPersonBoatCameraPosition.y + 2.0f, firstPersonBoatCameraPosition.z);

			firstPersonCamera->MoveFirstPerson(firstPersonBoatCameraPosition, false);
			firstPersonCamera->camera._at = boatFacingDirection;
			_view = firstPersonCamera->camera._view;
		}
		else if (cameraActive == 4)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			boatPosition.x = fCurrentBoatPosition._41;
			boatPosition.y = fCurrentBoatPosition._42;
			boatPosition.z = fCurrentBoatPosition._43;

			boatPosition = XMFLOAT3(boatPosition.x, boatPosition.y, boatPosition.z);

			thirdPersonBoatCameraPosition = XMFLOAT3(boatPosition.x, boatPosition.y + 20.0f, boatPosition.z - 15.0f);

			thirdPersonCamera->MoveThirdPerson(thirdPersonBoatCameraPosition, boatPosition, true);
			_view = thirdPersonCamera->camera._view;
		}
	}
	else if (GetAsyncKeyState(VK_DOWN)) // Back Key - Move Boat Backwards
	{
		playerBoat = playerBoat * XMMatrixTranslationFromVector(-boatFacingDirection / 200);

		if (cameraActive == 2)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			boatPosition.x = fCurrentBoatPosition._41;
			boatPosition.y = fCurrentBoatPosition._42 + 2.0f;
			boatPosition.z = fCurrentBoatPosition._43;

			boatPosition = XMFLOAT3(boatPosition.x, boatPosition.y, boatPosition.z);

			firstPersonCamera->MoveFirstPerson(boatPosition, false);
			firstPersonCamera->camera._at = boatFacingDirection;
			_view = firstPersonCamera->camera._view;
		}
		else if (cameraActive == 4)
		{
			XMStoreFloat4x4(&fCurrentBoatPosition, playerBoat);
			boatPosition.x = fCurrentBoatPosition._41;
			boatPosition.y = fCurrentBoatPosition._42;
			boatPosition.z = fCurrentBoatPosition._43;

			boatPosition = XMFLOAT3(boatPosition.x, boatPosition.y, boatPosition.z);

			thirdPersonBoatCameraPosition = XMFLOAT3(boatPosition.x, boatPosition.y + 20.0f, boatPosition.z - 15.0f);

			thirdPersonCamera->MoveThirdPerson(thirdPersonBoatCameraPosition, boatPosition, true);
			_view = thirdPersonCamera->camera._view;
		}
	}

	//
	// Object Mesh Controls
	//

	if (GetAsyncKeyState(VK_NUMPAD9) & 0x8000) // Numpad 9 Toggle Key
	{
		_currentState = _wireFrame;
	}
	else 
	{
		_currentState = _solidFrame;
	}

	//
	// Update Objects
	//

	// Boat Update Values
	XMStoreFloat4x4(&_world, playerBoat);

	// Water Update Values
	XMStoreFloat4x4(&_world2, XMMatrixTranslation(0.0f, -1.5f, 0.0f));

	// Rock Update Values
	XMStoreFloat4x4(&_rocks[0], XMMatrixTranslation(-50.0f, -6.0f, 67.0f));
	XMStoreFloat4x4(&_rocks[1], XMMatrixTranslation(-25.0f, -6.0f, 67.0f));
	XMStoreFloat4x4(&_rocks[2], XMMatrixTranslation(0.0f, -6.0f, 67.0f));
	XMStoreFloat4x4(&_rocks[3], XMMatrixTranslation(25.0f, -6.0f, 67.0f));
	XMStoreFloat4x4(&_rocks[4], XMMatrixTranslation(50.0f, -6.0f, 67.0f));
	XMStoreFloat4x4(&_rocks[5], XMMatrixTranslation(-50.0f, -7.0f, -67.0f));
	XMStoreFloat4x4(&_rocks[6], XMMatrixTranslation(-25.0f, -7.0f, -67.0f));
	XMStoreFloat4x4(&_rocks[7], XMMatrixTranslation(0.0f, -7.0f, -67.0f));
	XMStoreFloat4x4(&_rocks[8], XMMatrixTranslation(25.0f, -7.0f, -67.0f));
	XMStoreFloat4x4(&_rocks[9], XMMatrixTranslation(50.0f, -7.0f, -67.0f));
	XMStoreFloat4x4(&_rocks[10], XMMatrixTranslation(-70.0f, -7.0f, 55.0f));
	XMStoreFloat4x4(&_rocks[11], XMMatrixTranslation(-70.0f, -7.0f, 40.0f));
	XMStoreFloat4x4(&_rocks[12], XMMatrixTranslation(-70.0f, -7.0f, 25.0f));
	XMStoreFloat4x4(&_rocks[13], XMMatrixTranslation(-70.0f, -7.0f, 10.0f));
	XMStoreFloat4x4(&_rocks[14], XMMatrixTranslation(-70.0f, -7.0f, -5.0f));
	XMStoreFloat4x4(&_rocks[15], XMMatrixTranslation(-70.0f, -7.0f, -20.0f));
	XMStoreFloat4x4(&_rocks[16], XMMatrixTranslation(-70.0f, -7.0f, -35.0f));
	XMStoreFloat4x4(&_rocks[17], XMMatrixTranslation(-70.0f, -7.0f, -50.0f));
	XMStoreFloat4x4(&_rocks[18], XMMatrixTranslation(-70.0f, -7.0f, -65.0f));
	XMStoreFloat4x4(&_rocks[19], XMMatrixTranslation(70.0f, -7.0f, 55.0f));
	XMStoreFloat4x4(&_rocks[20], XMMatrixTranslation(70.0f, -7.0f, 40.0f));
	XMStoreFloat4x4(&_rocks[21], XMMatrixTranslation(70.0f, -7.0f, 25.0f));
	XMStoreFloat4x4(&_rocks[22], XMMatrixTranslation(70.0f, -7.0f, 10.0f));
	XMStoreFloat4x4(&_rocks[23], XMMatrixTranslation(70.0f, -7.0f, -5.0f));
	XMStoreFloat4x4(&_rocks[24], XMMatrixTranslation(70.0f, -7.0f, -20.0f));
	XMStoreFloat4x4(&_rocks[25], XMMatrixTranslation(70.0f, -7.0f, -35.0f));
	XMStoreFloat4x4(&_rocks[26], XMMatrixTranslation(70.0f, -7.0f, -50.0f));
	XMStoreFloat4x4(&_rocks[27], XMMatrixTranslation(70.0f, -7.0f, -65.0f));
	
	XMStoreFloat4x4(&_world3, XMMatrixScaling(120.0f, 120.0f, 120.0f)* XMMatrixTranslation(0.0f, 5.0f, 0.0f)* XMMatrixRotationY(-t / 10));
}


void Application::Draw()
{
	//
	// Clear the back buffer
	//

	float ClearColor[4] = { 0.0f, 0.125f, 0.5f, 1.0f }; // red,green,blue,alpha
	_pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	XMMATRIX world = XMLoadFloat4x4(&_world);
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

	//
	// Update variables
	//

	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
	cb.diffuseLight = diffuseLight;
	cb.diffuseMtrl = diffuseMaterial;
	cb.ambientLight = ambientLight;
	cb.ambientMtrl = ambientMaterial;
	cb.LightVecW = lightDirection;
	cb.SpecularMtrl = specularMaterial;
	cb.SpecularLight = specularLight;
	cb.SpecularPower = specularPower;
	cb.EyePosW = eyePosW;

	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->RSSetState(_currentState);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	//
	// Drawing Objects
	//

	// Draw Boat
	_pImmediateContext->IASetVertexBuffers(0, 1, &objMeshDataBoat.VertexBuffer, &objMeshDataBoat.VBStride, &objMeshDataBoat.VBOffset);
	_pImmediateContext->IASetIndexBuffer(objMeshDataBoat.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV); //Textures
	_pImmediateContext->DrawIndexed(objMeshDataBoat.IndexCount, 0, 0);

	// Draw Water
	_pImmediateContext->IASetVertexBuffers(0, 1, &objMeshDataWater.VertexBuffer, &objMeshDataWater.VBStride, &objMeshDataWater.VBOffset);
	_pImmediateContext->IASetIndexBuffer(objMeshDataWater.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRVWater); //Textures

	_pImmediateContext->VSSetShader(_pVertexShaderWater, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShaderWater, nullptr, 0);

	world = XMLoadFloat4x4(&_world2);
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->DrawIndexed(objMeshDataWater.IndexCount, 0, 0);

	// Drawing Rocks
	_pImmediateContext->IASetVertexBuffers(0, 1, &objMeshDataRock.VertexBuffer, &objMeshDataRock.VBStride, &objMeshDataRock.VBOffset);
	_pImmediateContext->IASetIndexBuffer(objMeshDataRock.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRVRock); //Textures
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

	for (int i = 0; i < 28; i++)
	{
		world = XMLoadFloat4x4(&_rocks[i]);
		cb.mWorld = XMMatrixTranspose(world);
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
		_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
		_pImmediateContext->DrawIndexed(objMeshDataRock.IndexCount, 0, 0);
	}


	// Sky Box Values
	_pImmediateContext->IASetVertexBuffers(0, 1, &objMeshDataSky.VertexBuffer, &objMeshDataSky.VBStride, &objMeshDataSky.VBOffset);
	_pImmediateContext->IASetIndexBuffer(objMeshDataSky.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	_pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRVSky); //Textures

	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

	world = XMLoadFloat4x4(&_world3);
	cb.mWorld = XMMatrixTranspose(world);
	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->DrawIndexed(objMeshDataSky.IndexCount, 0, 0);

	//
	// Present our back buffer to our front buffer
	//
	_pSwapChain->Present(0, 0);
}