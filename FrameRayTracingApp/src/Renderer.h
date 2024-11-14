#pragma once

#include <memory>

#include "Image.h"
#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <glm/glm.hpp>

class Renderer
{
public:
	struct Settings
	{
		bool Accumulate = true;
		bool FastAccumulate = false;
	};

public:
	Renderer() = default;

	void Render(const Scene& scene, const Camera& camera);
	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Frame::Image> GetFinalImage() const { return m_FinalImage; }

	void ResetFrameIndex() { m_FrameIndex = 1; }
	Settings& GetSettings() { return m_Settings; }

private:
	struct HitPayload
	{
		float HitDistance;
		glm::vec3 WorldPosition;
		glm::vec3 WorldNormal;

		bool FrontFace;

		void SetFaceNormal(const Ray& ray, const glm::vec3& outwardNormal)
		{
			FrontFace = glm::dot(ray.Direction, outwardNormal) < 0;
			WorldNormal = FrontFace ? outwardNormal : -outwardNormal;
		}

		int ObjectIndex;
	};

	glm::vec4 PerPixel(uint32_t x, uint32_t y); // Ray gen shader

	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);

private:
	Settings m_Settings;

	std::shared_ptr<Frame::Image> m_FinalImage;

	std::vector<uint32_t> m_ImageHorizontalIterator, m_ImageVerticalIterator;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	uint32_t* m_ImageData = nullptr;
	glm::vec4* m_AccumulationData = nullptr;

	uint32_t m_FrameIndex = 1;
};

