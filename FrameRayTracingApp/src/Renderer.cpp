#include "Renderer.h"

#include <execution>

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

	static uint32_t PCG_Hash(uint32_t input)
	{
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state)  * 277803737u;

		return (word >> 22u) ^ word;
	}

	static float RandomFloat(uint32_t& seed)
	{
		seed = PCG_Hash(seed);
		return (float)seed / (float)std::numeric_limits<uint32_t>::max();	
	}

	static glm::vec3 InUnitSphere(uint32_t& seed)
	{
		return glm::normalize(glm::vec3(RandomFloat(seed) * 2.0f - 1.0f, RandomFloat(seed) * 2.0f - 1.0f, RandomFloat(seed) * 2.0f - 1.0f));
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

	m_ImageHorizontalIterator.resize(width);
	m_ImageVerticalIterator.resize(height);

	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIterator[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIterator[i] = i;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	glm::vec3 light(0.0f);
	glm::vec3 throughput(1.0f);

	// Custom random method variables
	uint32_t seed = x + y * m_FinalImage->GetWidth();
	seed *= m_FrameIndex;

	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		seed += i;

		Renderer::HitPayload payload = TraceRay(ray);
		if (payload.HitDistance < 0.0f)
		{
			glm::vec3 skyColor(0.6f, 0.7f, 0.9f);
			light += skyColor * throughput;
			break;
		}

		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];

		throughput *= material.Albedo;
		light += material.GetEmission();

		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;
		if (!m_Settings.FastAccumulate)
			/*ray.Direction = glm::normalize(payload.WorldNormal + material.Roughness * Frame::Random::Vec3(-0.5f, 0.5f));*/
			ray.Direction = glm::normalize(payload.WorldNormal + Frame::Random::InUnitSphere());
		else
			/*ray.Direction = glm::normalize(payload.WorldNormal + material.Roughness * Frame::Random::Vec3(-0.5f, 0.5f));*/
			ray.Direction = glm::normalize(payload.WorldNormal + Utils::InUnitSphere(seed));
	}

	return glm::vec4(light, 1);
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

#define MT
#ifdef MT
	std::for_each(std::execution::par, m_ImageVerticalIterator.begin(), m_ImageVerticalIterator.end(),
		[this](uint32_t y)
		{
			std::for_each(std::execution::par, m_ImageHorizontalIterator.begin(), m_ImageHorizontalIterator.end(),
				[this, y](uint32_t x)
				{
					glm::vec4 color = PerPixel(x, y);
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

					glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];
					accumulatedColor /= (float)m_FrameIndex;


					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRBGA(accumulatedColor);
				});
		});
#else
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
#endif

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
