#include "Application.h"
#include "Entrypoint.h"

#include "Image.h"
#include "Random.h"
#include "Timer.h"

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

		if(m_Image)
			ImGui::Image((unsigned long long)m_Image->GetDescriptorSet(), { (float)m_Image->GetWidth(), (float)m_Image->GetHeight() });

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		if (!m_Image || m_ViewPortWidth != m_Image->GetWidth() || m_ViewPortHeight != m_Image->GetHeight())
		{
			m_Image = std::make_shared<Image>(m_ViewPortWidth, m_ViewPortHeight, ImageFormat::RGBA);
			delete[] m_ImageData;

			m_ImageData = new uint32_t[m_ViewPortWidth * m_ViewPortHeight];
		}

		for (uint32_t i = 0; i < m_ViewPortWidth * m_ViewPortHeight; i++)
		{
			m_ImageData[i] = Random::UInt();
			m_ImageData[i] |= 0xff000000;
		}

		m_Image->SetData(m_ImageData);
		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	std::shared_ptr<Image> m_Image;
	uint32_t* m_ImageData = nullptr;

	uint32_t m_ViewPortWidth = 0, m_ViewPortHeight = 0;
	float m_LastRenderTime = 0.0f;
};

Frame::Application* Frame::CreateApplication(int argc, char** argv)
{
	ApplicationSettings settings;
	settings.Name = "Test";
	

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