#include "Camera.h"
#include "../Base/BaseObject.h"
#include "../class/UniformData.h"

bool Camera::Init(const CameraInfo& info, const std::shared_ptr<Camera>& pCamera)
{
	if (!BaseComponent::Init(pCamera))
		return false;

	SetCameraInfo(info);
	return true;
}

std::shared_ptr<Camera> Camera::Create(const CameraInfo& info)
{
	std::shared_ptr<Camera> pCamera = std::make_shared<Camera>();
	if (pCamera.get() && pCamera->Init(info, pCamera))
		return pCamera;
	return nullptr;
}

void Camera::Update()
{
}

void Camera::LateUpdate()
{
	UpdateViewMatrix();
	UpdateProjMatrix();
}

void Camera::UpdateViewMatrix()
{
	if (m_pObject.expired())
		return;

	UniformData::GetInstance()->GetPerFrameUniforms()->SetViewMatrix(m_pObject.lock()->GetWorldTransform().Inverse());
}

void Camera::UpdateProjMatrix()
{
	if (!m_projDirty)
		return;

	float tanFOV2 = std::tanf(m_cameraInfo.fov / 2.0f);

	Matrix4f proj;
	proj.x0 = 1.0f / (m_cameraInfo.aspect * tanFOV2 * m_cameraInfo.near);
	proj.y1 = 1.0f / (tanFOV2 * m_cameraInfo.near);
	proj.z2 = -(m_cameraInfo.far + m_cameraInfo.near) / (m_cameraInfo.far - m_cameraInfo.near);
	proj.z3 = -1.0f;
	proj.w2 = -2.0f * m_cameraInfo.near * m_cameraInfo.far / (m_cameraInfo.far - m_cameraInfo.near);
	proj.w3 = 0.0f;

	UniformData::GetInstance()->GetGlobalUniforms()->SetProjectionMatrix(proj);
}

void Camera::UpdateCameraPosition()
{
	UniformData::GetInstance()->GetPerFrameUniforms()->SetCameraPosition(GetObject()->GetWorldPosition());
}

void Camera::SetFOV(float new_fov)
{
	m_cameraInfo.fov = new_fov; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetAspect(float new_aspect)
{ 
	m_cameraInfo.aspect = new_aspect; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetNearPlane(float new_near_plane)
{ 
	m_cameraInfo.near = new_near_plane; 
	UpdateCameraSupplementInfo();

	m_projDirty = true;
}

void Camera::SetCameraInfo(const CameraInfo& info) 
{ 
	m_cameraInfo = info; 
	UpdateCameraSupplementInfo();

	m_projDirty = true; 
}

void Camera::UpdateCameraSupplementInfo()
{
	float tanFOV2 = std::tanf(m_cameraInfo.fov / 2.0f);

	//update horizontal fov
	//tan(fovh/2) = asp*tan(fov/2)       1)
	//fovh = 2*arctan(asp*tan(fov/2))    2)
	m_fovH = std::atanf(m_cameraInfo.aspect * tanFOV2) * 2.0f;
}