#include "Input.h"

#include "Application.h"

#include <GLFW/glfw3.h>

namespace Frame
{
	bool Input::IsKeyDown(int keyCode)
	{
		GLFWwindow* windowHandle = Application::GetApp().GetWindowHandle();
		int state = glfwGetKey(windowHandle, keyCode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonDown(int mouseButton)
	{
		GLFWwindow* windowHandle = Application::GetApp().GetWindowHandle();
		int state = glfwGetMouseButton(windowHandle, mouseButton);
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePosition()
	{
		GLFWwindow* windowHandle = Application::GetApp().GetWindowHandle();

		double x, y;
		glfwGetCursorPos(windowHandle, &x, &y);
		return { (float)x, (float)y };
	}

	void Input::SetCursorMode(CursorMode mode)
	{
		GLFWwindow* windowHandle = Application::GetApp().GetWindowHandle();
		glfwSetInputMode(windowHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}
}
