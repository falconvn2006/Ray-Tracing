#pragma once

#ifdef FRAME_PLATFORM_WINDOWS

extern Frame::Application* Frame::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

namespace Frame
{
	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning)
		{
			Frame::Application* app = Frame::CreateApplication(argc, argv);
			app->Run();
			delete app;
		}

		return 0;
	}
}

#ifdef FRAME_DIST

#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return Frame::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Frame::Main(argc, argv);
}

#endif // FRAME_DIST

#endif // FRAME_PLATFORM_WINDOWS
