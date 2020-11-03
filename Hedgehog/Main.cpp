
// TODO: Applications using the Hedgehog engine should just include some Hedgehog.h header
//       and then create a concrete application class inheriting from the Hedgehog Application class
// TODECIDE: Should the main function/entry point be a part of the engine or should it be up to the application (as it is here)?
#include <Application/Application.h>

#include <Layer/Layer.h>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
//#include <spdlog/spdlog.h>
//#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>


class ExampleLayer : public Layer
{
public:
	ExampleLayer(const std::string& name, bool enable = true) : Layer(name, enable) { }

	void OnUpdate() override
	{
		//printf("%s: OnUpdate called\n", name.c_str());
	}

	void OnMessage(const Message& message) override
	{
		printf("%s: OnMessage called\n", name.c_str());
		std::cout << message.ToString() << std::endl;
	}
};

class Sandbox : public Application
{
public:
	Sandbox(HINSTANCE hInstance) : Application(hInstance)
	{
		layers.Push(new ExampleLayer("1st Example Layer"));
		layers.PushOverlay(new ExampleLayer("1st Example Overlay"));
		layers.Push(new ExampleLayer("2nd Example Layer"));
		layers.Push(new ExampleLayer("3rd Example Layer", false));
	}

	~Sandbox()
	{
		Layer* layer;
		while (layer = layers.TopOverlay())
		{
			layers.PopOverlay();
			delete layer;
		}
		while (layer = layers.Top())
		{
			layers.Pop();
			delete layer;
		}
	}
};



// Main function for SubSystem Console
int main(int argc, char* argv[])
{
	HINSTANCE hInstance = GetModuleHandle(NULL);
	Sandbox app(hInstance);
	app.Run();

	return 0;
}


// Main function for SubSystem Windows
// This SubSystem doesn't open a console window, it needs to be opened explicitely
//int WINAPI wWinMain(_In_ HINSTANCE hInstance,
//					_In_opt_ HINSTANCE hPrevInstance,
//					_In_ LPWSTR lpCmdLine,
//					_In_ int nShowCmd)
//{
//	FILE* fp;
//
//	AllocConsole();
//	freopen_s(&fp, "CONIN$", "r", stdin);
//	freopen_s(&fp, "CONOUT$", "w", stdout);
//	freopen_s(&fp, "CONOUT$", "w", stderr);
//
//	Application app(hInstance);
//
//	app.Run();
//
//	return EXIT_SUCCESS;
//}
