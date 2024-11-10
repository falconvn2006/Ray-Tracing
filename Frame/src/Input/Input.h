#pragma once

#include "KeyCodes.h"
#include <glm/glm.hpp>

namespace Frame
{
	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

	class Input
	{
	public:
		static bool IsKeyDown(int keyCode);
		static bool IsMouseButtonDown(int mouseButton);

		static glm::vec2 GetMousePosition();

		static void SetCursorMode(CursorMode mode);
	};
}

