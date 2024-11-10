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
	Renderer() = default;

	void Render(const Scene& scene, const Camera& camera);
	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Frame::Image> GetFinalImage() const { return m_FinalImage; }
private:
	glm::vec4 TraceRay(const Scene& scene, const Ray& ray);
private:
	std::shared_ptr<Frame::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	// Temporary for controlling the sphere
	glm::vec3 m_SphereColor;
};

