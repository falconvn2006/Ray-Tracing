#include "Application.h"
#include "Entrypoint.h"

#include "Image.h"
#include "Timer.h"

#include "Renderer.h"

using namespace Frame;

class ExampleLayer : public Frame::Layer
{
public:
	virtual void OnImGUIRender() override
	{
		ImGui::Begin("Settings");
		
		ImGui::Text("Last Render time: %.3fms", m_LastRenderTime);

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");

		m_ViewPortWidth = ImGui::GetContentRegionAvail().x;
		m_ViewPortHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();

		if(image)
			ImGui::Image((unsigned long long)image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() }, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		// Call renderer resize
		m_Renderer.OnResize(m_ViewPortWidth, m_ViewPortHeight);
		// Call renderer render
		m_Renderer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;

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