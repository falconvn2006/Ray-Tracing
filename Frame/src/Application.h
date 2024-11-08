#pragma once

#include "Layer.h"

// Since this is a simple app we just include all the necessary header files where we need.
#include <string>
#include <vector>
#include <memory>
#include <functional>

#include "imgui.h"
#include "vulkan/vulkan.h"

void check_vk_result(VkResult result);

// Window struct to hold the window data
struct GLFWwindow;

namespace Frame
{
	// Custom application struct for setting up the app
	struct ApplicationSettings
	{
		std::string Name = "Frame App";

		uint32_t Width = 1600;
		uint32_t Height = 900;
	};

	class Application
	{
	public:
		Application(const ApplicationSettings& settings = ApplicationSettings());
		~Application();

		static Application& GetApp();

		void Run();
		void SetMenuBarCallback(const std::function<void()>& menuBarCallback) { m_MenuBarCallback = menuBarCallback; }

		// Make a template so we can put the Layer into a shared pointer easier without to convert from else where
		template<typename T>
		void PushLayer()
		{
			static_assert(std::is_base_of<Layer, T>::value, "Object is not a child/inheritant from the Layer class!");
			m_LayerStack.emplace_back(std::make_shared<T>())->OnAttach();
		}

		void PushLayer(const std::shared_ptr<Layer>& layer) { m_LayerStack.emplace_back(layer); layer->OnAttach(); }

		void Close();
		
		float GetTime();

		GLFWwindow* GetWindowHandle() const { return m_WindowHandle; }

		static VkInstance GetInstance();
		static VkPhysicalDevice GetPhysicalDevice();
		static VkDevice GetDevice();

		static VkCommandBuffer GetCommandBuffer(bool begin);
		static void FlushCommandBuffer(VkCommandBuffer cmdbuffer);

		static void SubmitResourceFree(std::function<void()>&& func);

	private:
		void Init();
		void Shutdown();

	private:
		ApplicationSettings m_AppSettings;
		GLFWwindow* m_WindowHandle = nullptr;
		bool m_Running = false;

		float m_TimeStep = 0.0f;
		float m_FrameTime = 0.0f;
		float m_LastFrameTime = 0.0f;

		std::vector<std::shared_ptr<Layer>> m_LayerStack;
		std::function<void()> m_MenuBarCallback;
	};

	// Implemented by the Client
	Application* CreateApplication(int argc, char** argv);
}


