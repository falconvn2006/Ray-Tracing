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

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 finalColor(0.0f);
	float multiplier = 1.0f;

	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor(0.6f, 0.7f, 0.9f);
			finalColor += skyColor * multiplier;
			break;
		}

		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));
		float d = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f);

		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

		glm::vec3 sphereColor = material.Albedo;
		sphereColor *= d;

		finalColor += sphereColor * multiplier;

		multiplier *= 0.7f;

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction,
			payload.WorldNormal + material.Roughness * Frame::Random::Vec3(-0.5f, 0.5f));
	}

	return glm::vec4(finalColor, 1);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	const Sphere& closetSphere = m_ActiveScene->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closetSphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance; // Kinda the world space position
	payload.WorldNormal = glm::normalize(payload.WorldPosition); // Kinda the world space normal
	payload.WorldPosition += closetSphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1.0f;
	return payload;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	// Render very pixel

	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

	// Calculate aspect ratio
	/*float aspectRatio = m_FinalImage->GetWidth() / (float)m_FinalImage->GetHeight();*/

	for(uint32_t y = 0;  y < m_FinalImage->GetHeight(); y++)
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
			accumulatedColor /= (float)m_FrameIndex;


			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRBGA(accumulatedColor);
		}

	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}

Renderer::HitPayload Renderer::TraceRay( const Ray& ray)
{ 
	int closetSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();
	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];

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
		if (t.second > 0.0f && t.second < hitDistance)
		{
			hitDistance = t.second;
			closetSphere = (int)i;
		}
	}

	if (closetSphere < 0)
		return Miss(ray);

	return ClosestHit(ray, hitDistance, closetSphere);
}
