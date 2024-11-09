#pragma once

#include <memory>
#include "Image.h"

#include <glm/glm.hpp>

class Renderer
{
public:
	Renderer() = default;

	void Render();
	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Frame::Image> GetFinalImage() const { return m_FinalImage; }
private:
	uint32_t PerPixel(glm::vec2 coord);
private:
	std::shared_ptr<Frame::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;
};

