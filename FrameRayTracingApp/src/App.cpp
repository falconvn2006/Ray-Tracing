#include "Application.h"
#include "Entrypoint.h"

class ExampleLayer : public Frame::Layer
{
public:
	virtual void OnImGUIRender() override
	{
		ImGui::Begin("Window");
		
		ImGui::Button("Press me!");

		ImGui::End();

		ImGui::ShowDemoWindow();
	}
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