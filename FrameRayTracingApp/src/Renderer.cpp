#include "Renderer.h"

#include "Random.h"

namespace Utils
{
	static uint32_t ConvertToRBGA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;

		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// Don't need to resize the image
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);	
	}
	else
	{
		m_FinalImage = std::make_shared<Frame::Image>(width, height, Frame::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	// Render very pixel

	// Calculate aspect ratio
	float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();

	for(uint32_t y = 0;  y < m_FinalImage->GetHeight(); y++)
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec2 coord = { (float)x / (float)m_FinalImage->GetWidth(), (float)y / (float)m_FinalImage->GetHeight() };
			coord = coord * 2.0f - 1.0f; // Remaping the coordinate from (0 -> 1) to (-1 -> 1)
			// Apply the aspect ratio
			coord.x *= aspectRatio;

			glm::vec4 color = PerPixel(coord);

			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));

			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRBGA(color);
		}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	uint32_t r = (uint32_t)(coord.x * 255.0f);
	uint32_t g = (uint32_t)(coord.y * 255.0f);

	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f); // Camera origin
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	float radius = 0.5f;

	//rayDirection = glm::normalize(rayDirection);

	// Ray and circle intersection equation
	// a : origin of the ray
	// b : direction of the ray
	// r : radius of the circle
	// t : hit distance
	// (bx^2 + by^2) + 2(ax*bx + ay*by)t + (ax^2 + ay^2 - r^2) = 0

	// These variables are ax^2 + bx + c = 0
	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// Discriminant calculation
	float disc = b * b - 4 * a * c;

	if (disc < 0.0f)
		return glm::vec4(0, 0, 0, 1);

	// Solution for the quadratic equation above
	std::pair<float, float> t = std::make_pair((-b + glm::sqrt(disc)) / (2.0f * a), (-b - glm::sqrt(disc)) / (2.0f * a));

	glm::vec3 hitPos = rayOrigin + rayDirection * t.second;
	glm::vec3 normal = glm::normalize(hitPos);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

	float d = glm::max(glm::dot(normal, -lightDir), 0.0f);

	glm::vec3 sphereColor = m_SphereColor;
	sphereColor *= d;
	return glm::vec4(sphereColor, 1);
}
