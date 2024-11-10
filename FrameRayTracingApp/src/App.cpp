#include "Application.h"
#include "Entrypoint.h"

#include "Image.h"
#include "Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Frame;

class ExampleLayer : public Frame::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f)
	{
		Material& whiteSphere = m_Scene.Materials.emplace_back();
		whiteSphere.Albedo = glm::vec3(1, 1, 1);
		whiteSphere.Roughness = 0.0f;

		Material& blueSphere = m_Scene.Materials.emplace_back();
		blueSphere.Albedo = glm::vec3(0.2f, 0.3f, 1.0f);
		blueSphere.Roughness = 0.1f;

		m_Scene.Spheres.push_back(Sphere{ {0.0f, 0.0f, 0.0f}, 1.0f, 0});
		m_Scene.Spheres.push_back(Sphere{ {0.0f, -101.0f, 0.0f}, 100.0f, 1 });;
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
	}

	virtual void OnImGUIRender() override
	{
		ImGui::Begin("Settings");

		ImGui::Text("Last Render time: %.3fms", m_LastRenderTime);

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset"))
			m_Renderer.ResetFrameIndex();

		ImGui::End();

		ImGui::Begin("Scene");

		for (int i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			ImGui::Text("Sphere %d properties:", i);

			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);

			ImGui::DragInt("Material ID", &sphere.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);

			ImGui::Separator();

			ImGui::PopID();
		}

		for (int i = 0; i < m_Scene.Materials.size(); i++)
		{
			ImGui::PushID(i);

			ImGui::Text("Material properties ID: %d", i);

			ImGui::ColorEdit3("Albedo", glm::value_ptr(m_Scene.Materials[i].Albedo));
			ImGui::DragFloat("Roughness", &m_Scene.Materials[i].Roughness, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &m_Scene.Materials[i].Metallic, 0.05f, 0.0f, 1.0f);

			ImGui::Separator();

			ImGui::PopID();
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewPortWidth = ImGui::GetContentRegionAvail().x;
		m_ViewPortHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();

		if (image)
			ImGui::Image((unsigned long long)image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewPortWidth, m_ViewPortHeight);
		m_Camera.OnResize(m_ViewPortWidth, m_ViewPortHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;

	uint32_t m_ViewPortWidth = 0, m_ViewPortHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Frame::Application* Frame::CreateApplication(int argc, char** argv)
{
	ApplicationSettings settings;
	settings.Name = "Frame Ray Tracer";
	

	Application* application = new Frame::Application(settings);
	application->PushLayer<ExampleLayer>();
	application->SetMenuBarCallback([application]()
	{
		if (ImGui::BeginMenu("Menu"))
		{
			if (ImGui::MenuItem("Exit"))
				application->Close();

			ImGui::EndMenu();
		}
	});

	return application;
}