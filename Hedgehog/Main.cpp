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
	ExampleLayer() : Layer("Example Layer") { }

	void OnUpdate() override
	{
		//printf("Example Layer: OnUpdate called\n");
	}

	void OnMessage(const Message& message) override
	{
		printf("Example layer: OnMessage called\n");
		std::cout << message.ToString() << std::endl;
	}
};

class ExampleOverlay : public Layer
{
public:
	ExampleOverlay() : Layer("Example Overlay") {}

	void OnUpdate() override
	{
		//printf("Example overlay: OnUpdate called\n");
	}

	void OnMessage(const Message& message) override
	{
		printf("Example overlay: OnMessage called\n");
		std::cout << message.ToString() << std::endl;
	}
};

class Sandbox : public Application
{
public:
	Sandbox(HINSTANCE hInstance) : Application(hInstance)
	{
		ExampleLayer* layer1 = new ExampleLayer();
		layers.Push(layer1);
		layers.PushOverlay(new ExampleOverlay());
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
