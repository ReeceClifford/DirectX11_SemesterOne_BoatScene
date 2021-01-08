#include "Camera.h"
#include "Application.h"
#include "Structures.h"

Camera::Camera(XMFLOAT3 eye, XMFLOAT3 at, XMFLOAT3 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth, bool lookTo)
{
	camera._eye = XMLoadFloat3(&eye);
	camera._at = XMLoadFloat3(&at);
	camera._up = XMLoadFloat3(&up);

	camera._windowWidth = windowWidth;
	camera._windowHeight = windowHeight;
	camera._nearDepth = nearDepth;
	camera._farDepth = farDepth;

	if (lookTo) {
		SetLookAtMatrix();
	}
	else if (!lookTo)
	{
		SetLookToMatrix();
	}
}

void Camera::SetLookAtMatrix()
{
	XMStoreFloat4x4(&camera._view, XMMatrixLookAtLH(camera._eye, camera._at, camera._up));
	XMStoreFloat4x4(&camera._projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, camera._windowWidth / (FLOAT)camera._windowHeight, camera._nearDepth, camera._farDepth));
}

void Camera::SetLookToMatrix()
{
	XMStoreFloat4x4(&camera._view, XMMatrixLookToLH(camera._eye, camera._at,camera._up));
	XMStoreFloat4x4(&camera._projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, camera._windowWidth / (FLOAT)camera._windowHeight, camera._nearDepth, camera._farDepth));
}

void Camera::Update(bool lookTo)
{
	if(lookTo)
	XMStoreFloat4x4(&camera._view, XMMatrixLookAtLH(camera._eye, camera._at, camera._up));
	else
	XMStoreFloat4x4(&camera._view, XMMatrixLookToLH(camera._eye, camera._at, camera._up));
}

void Camera::MoveFirstPerson(XMFLOAT3 position, bool lookTo)
{
	camera._eye = XMLoadFloat3(&position);

	if (lookTo)
		Update(lookTo);
	else
		Update(lookTo);
}

void Camera::MoveThirdPerson(XMFLOAT3 position, XMFLOAT3 at, bool lookTo)
{
	camera._eye = XMLoadFloat3(&position);
	camera._at = XMLoadFloat3(&at);

	
	if (lookTo)
		Update(lookTo);
	else
		Update(lookTo);

}

Camera::~Camera()
{

}
