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

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	// Render very pixel

	// Calculate aspect ratio
	/*float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();*/

	Ray ray;
	ray.Origin = camera.GetPosition();

	for(uint32_t y = 0;  y < m_FinalImage->GetHeight(); y++)
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			// Apply the aspect ratio
			/*coord.x *= aspectRatio;*/

			ray.Direction = camera.GetRayDirections()[x + y * m_FinalImage->GetWidth()];
			glm::vec4 color = TraceRay(scene, ray);

			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));

			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRBGA(color);
		}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
	if (scene.Spheres.size() == 0)
		return glm::vec4(0, 0, 0, 1);

	const Sphere* closetSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();
	for (const Sphere& sphere : scene.Spheres)
	{
		glm::vec3 origin = ray.Origin - sphere.Position;

		// Ray and circle intersection equation
		// a : origin of the ray
		// b : direction of the ray
		// r : radius of the circle
		// t : hit distance
		// (bx^2 + by^2) + 2(ax*bx + ay*by)t + (ax^2 + ay^2 - r^2) = 0

		// These variables are ax^2 + bx + c = 0
		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		// Discriminant calculation
		float disc = b * b - 4 * a * c;

		if (disc < 0.0f)
			continue;

		// Solution for the quadratic equation above
		std::pair<float, float> t = std::make_pair((-b + glm::sqrt(disc)) / (2.0f * a), (-b - glm::sqrt(disc)) / (2.0f * a));
		if (t.second < hitDistance)
		{
			hitDistance = t.second;
			closetSphere = &sphere;
		}
	}

	if (closetSphere == nullptr)
		return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::vec3 origin = ray.Origin - closetSphere->Position;

	glm::vec3 hitPos = origin + ray.Direction * hitDistance;
	glm::vec3 normal = glm::normalize(hitPos);

	glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

	float d = glm::max(glm::dot(normal, -lightDir), 0.0f);

	glm::vec3 sphereColor = closetSphere->Albedo;
	sphereColor *= d;
	return glm::vec4(sphereColor, 1);
}
