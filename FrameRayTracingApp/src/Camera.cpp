#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


#include "Input/Input.h"

Camera::Camera(float verticalFOV, float nearClip, float farClip)
	: m_VerticalFOV(verticalFOV), m_NearClip(nearClip), m_FarClip(farClip)
{
	m_ForwardDirection = glm::vec3(0, 0, -1);
	m_Position = glm::vec3(0, 0, 3);
}

bool Camera::OnUpdate(float ts)
{
	glm::vec2 mousePos = Frame::Input::GetMousePosition();
	glm::vec2 delta = (mousePos - m_LastMousePosition) * 0.002f;
	m_LastMousePosition = mousePos;

	// The second way of moving the camera

	if (Frame::Input::IsKeyDown(FRAME_KEY_P))
		m_IsToggle = !m_IsToggle;

	if (!Frame::Input::IsMouseButtonDown(FRAME_MOUSE_BUTTON_2) && !m_IsToggle)
	{
		Frame::Input::SetCursorMode(Frame::CursorMode::Normal);
		return false;
	}

	Frame::Input::SetCursorMode(Frame::CursorMode::Locked);

	bool moved = false;

	constexpr glm::vec3 upDirection(0.f, 1.f, 0.f);
	glm::vec3 rightDirection = glm::cross(m_ForwardDirection, upDirection);

	float speed = 5.0f;

	// Movement
	if (Frame::Input::IsKeyDown(FRAME_KEY_W))
	{
		m_Position += m_ForwardDirection * speed * ts;
		moved = true;
	}
	else if (Frame::Input::IsKeyDown(FRAME_KEY_S))
	{
		m_Position -= m_ForwardDirection * speed * ts;
		moved = true;
	}

	if (Frame::Input::IsKeyDown(FRAME_KEY_A))
	{
		m_Position -= rightDirection * speed * ts;
		moved = true;
	}
	else if (Frame::Input::IsKeyDown(FRAME_KEY_D))
	{
		m_Position += rightDirection * speed * ts;
		moved = true;
	}

	if (Frame::Input::IsKeyDown(FRAME_KEY_Q))
	{
		m_Position -= upDirection * speed * ts;
		moved = true;
	}
	else if (Frame::Input::IsKeyDown(FRAME_KEY_E))
	{
		m_Position += upDirection * speed * ts;
		moved = true;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		float pitchDelta = delta.y * GetRotationSpeed(); // Rotate around the x axis
		float yawDelta = delta.x * GetRotationSpeed(); // Rotate around the y axis

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, upDirection)));
		m_ForwardDirection = glm::rotate(q, m_ForwardDirection);

		moved = true;
	}

	if (moved)
	{
		ReCalculateView();
		ReCalculateRayDirections();
	}

	return moved;
}

void Camera::OnResize(uint32_t width, uint32_t height)
{
	if (width == m_ViewPortWidth && height == m_ViewPortHeight)
		return;

	m_ViewPortWidth = width;
	m_ViewPortHeight = height;

	ReCalculateProjection();
	ReCalculateRayDirections();
}

float Camera::GetRotationSpeed()
{
	return 0.3f;
}

void Camera::ReCalculateProjection()
{
	m_Projection = glm::perspectiveFov(glm::radians(m_VerticalFOV), (float)m_ViewPortWidth, (float)m_ViewPortHeight, m_NearClip, m_FarClip);
	m_InverseProjection = glm::inverse(m_Projection);
}

void Camera::ReCalculateView()
{
	m_View = glm::lookAt(m_Position, m_Position + m_ForwardDirection, glm::vec3(0, 1, 0));
	m_InverseView = glm::inverse(m_View);
}

void Camera::ReCalculateRayDirections()
{
	m_RayDirections.resize(m_ViewPortWidth * m_ViewPortHeight);

	for(uint32_t y = 0; y < m_ViewPortHeight; y++)
		for (uint32_t x = 0; x < m_ViewPortWidth; x++)
		{
			glm::vec2 coord = { (float)x / (float)m_ViewPortWidth, (float)y / (float)m_ViewPortHeight };
			coord = coord * 2.0f - 1.0f; // Remaping the coordinate from (0 -> 1) to (-1 -> 1)

			glm::vec4 target = m_InverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(m_InverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World space
			m_RayDirections[x + y * m_ViewPortWidth] = rayDirection;
		}
}
