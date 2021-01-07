
// TODO: Applications using the Hedgehog engine should just include some Hedgehog.h header
//       and then create a concrete application class inheriting from the Hedgehog Application class
//       altough so far everything needed is in the Application anyway
// TODECIDE: Should the main function/entry point be a part of the engine or should it be up to the application (as it is here)?
#include <Application/Application.h>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
//#include <spdlog/spdlog.h>
//#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>


class ExampleLayer : public Layer
{
public:
	ExampleLayer(bool enable = true) :
		Layer("Example Triangle Layer", enable),
		camera(56.0f, aspectRatio, 0.01f, 25.0f) // camera space, +z goes into the screen
		//camera(-aspectRatio, aspectRatio, -1.0f, 1.0f, 0.01f, 25.0f)
	{
		camera.SetPosition({ 1.0f, 1.0f, 3.0f }); // world space, +z goes out of the screen
		camera.SetRotation({ -10.0f, 20.0f, 0.0f });
	}

	void OnUpdate(const std::chrono::duration<double, std::milli>& duration) override
	{
		// Poll WASD input
		if (GetKeyState(0x44) < 0) // 'D'
		{
			xOffset++;
		}

		if (GetKeyState(0x41) < 0) // 'A'
		{
			xOffset--;
		}

		if (GetKeyState(0x57) < 0) // 'W'
		{
			zOffset--;
		}

		if (GetKeyState(0x53) < 0) // 'S'
		{
			zOffset++;
		}

		if (GetKeyState(0x6B) < 0) // '+'
		{
			zRotation++;
		}

		if (GetKeyState(0x6D) < 0) // '-'
		{
			zRotation--;
		}

		if (GetKeyState(0x45) < 0) // 'E'
		{
			zRotation--;
		}

		if (GetKeyState(0x51) < 0) // 'Q'
		{
			zRotation++;
		}

		if (GetKeyState(VK_SPACE) < 0)
		{
			camera.SetPosition({ 0.0f, 0.0f, 3.0f });
			camera.SetRotation({ 0.0f, 0.0f, 0.0f });
		}

		camera.Move(glm::vec3(xOffset * movementSpeed * (float)duration.count(), yOffset * scrollSpeed, zOffset * movementSpeed * (float)duration.count()));
		camera.Rotate(glm::vec3(mouseSpeed * xRotation, mouseSpeed * yRotation, zRotation * rotationSpeed * (float)duration.count()));

		xOffset = 0;
		yOffset = 0;
		zOffset = 0;

		xRotation = 0;
		yRotation = 0;
		zRotation = 0;

		// TODO just a tempporary example to use the Renderer API, creating and uploading the data with each frame is not the way
		std::shared_ptr<VertexArray> vertexArray;
		vertexArray.reset(VertexArray::Create());

		// Vertex Buffer
		BufferLayout vertexBufferLayout =
		{
			{ ShaderDataType::Float4, "a_position" },
			{ ShaderDataType::Float4, "a_color" }
		};

		float vertices[] =
		{
			// 1x1x1 cube centered around the origin (0, 0, 0)
			// front face - white with some red on the bottom
			-0.5f,  0.5f,  0.5f, 1.0f,	1.0f, 1.0f, 1.0f, 1.0f, // top left
			 0.5f,  0.5f,  0.5f, 1.0f,	1.0f, 1.0f, 1.0f, 1.0f, // top right
			-0.5f, -0.5f,  0.5f, 1.0f,	1.0f, 0.8f, 0.8f, 1.0f, // bottom left
			 0.5f, -0.5f,  0.5f, 1.0f,	1.0f, 0.8f, 0.8f, 1.0f, // bottom right
			 // back face - black with some red on the bottom
			-0.5f,  0.5f, -0.5f, 1.0f,	0.0f, 0.0f, 0.0f, 1.0f, // top left
			 0.5f,  0.5f, -0.5f, 1.0f,	0.0f, 0.0f, 0.0f, 1.0f, // top right
			-0.5f, -0.5f, -0.5f, 1.0f,	0.2f, 0.0f, 0.0f, 1.0f, // bottom left
			 0.5f, -0.5f, -0.5f, 1.0f,	0.2f, 0.0f, 0.0f, 1.0f, // bottom right
		};

		std::shared_ptr<VertexBuffer> vertexBuffer;
		vertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));
		vertexBuffer->SetLayout(vertexBufferLayout);

		vertexArray->AddVertexBuffer(vertexBuffer);

		// Index Buffer
		unsigned int indices[] = { 0,2,1, 1,2,3, 4,5,7, 4,7,6, 2,6,3, 3,6,7, 0,5,4, 0,1,5, 1,3,7, 1,7,5, 0,4,2, 2,4,6 };
		std::shared_ptr<IndexBuffer> indexBuffer;
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(unsigned int)));

		vertexArray->AddIndexBuffer(indexBuffer);

		std::string vertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLExampleVertexShader.glsl";
		std::string fragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLExamplePixelShader.glsl";

		std::shared_ptr<Shader> shader;
		shader.reset(Shader::Create(vertexSrc, fragmentSrc));

		Renderer::BeginScene(camera);
		{
			Renderer::Submit(shader, vertexArray);
		}
		Renderer::EndScene();
	}

	void OnGuiUpdate() override
	{
		ImGui::Begin("Camera");

		glm::vec3 position = camera.GetPosition();
		glm::vec3 rotation = camera.GetRotation();
		ImGui::Text("Position: %f %f %f", position.x, position.y, position.z);
		ImGui::Text("Rotation: %f %f %f", rotation.x, rotation.y, rotation.z);

		ImGui::End();
	}

	void OnMessage(const Message& message) override
	{
		printf("%s: OnMessage called\n", name.c_str());
		std::cout << message.ToString() << std::endl;

		if (message.GetMessageType() == MessageType::MouseScrolled)
		{
			const MouseScrollMessage& mouseScrollMessage = dynamic_cast<const MouseScrollMessage&>(message);

			yOffset += mouseScrollMessage.GetDistance();
		}

		if (message.GetMessageType() == MessageType::MouseMoved)
		{
			const MouseMoveMessage& mouseMoveMessage = dynamic_cast<const MouseMoveMessage&>(message);

			if (lastX == 0 && lastY == 0)
			{
				lastX = mouseMoveMessage.GetX();
				lastY = mouseMoveMessage.GetY();
			}
			else
			{
				yRotation -= ((float)mouseMoveMessage.GetX() - (float)lastX);
				xRotation -= ((float)mouseMoveMessage.GetY() - (float)lastY);

				lastX = mouseMoveMessage.GetX();
				lastY = mouseMoveMessage.GetY();
			}
		}
	}

private:
	float aspectRatio = 1264.0f / 681.0f;
	PerspectiveCamera camera;
	//OrthographicCamera camera;

	int lastX = 0;
	int lastY = 0;

	float xOffset = 0;
	float yOffset = 0;
	float zOffset = 0;

	float xRotation = 0;
	float yRotation = 0;
	float zRotation = 0;

	float movementSpeed = 5.0f / 1000.0f; // units/ms
	float rotationSpeed = 180.0f / 1000.0f; // deg/ms
	float mouseSpeed = 135.0f / 681.0f; // deg/px
	float scrollSpeed = 0.25; // units/mousestep
};

class ExampleOverlay : public Layer
{
public:
	ExampleOverlay(const std::string& name, bool enable = true) : Layer(name, enable) { }

	void OnGuiUpdate() override
	{
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
			RenderCommand::SetClearColor({ clear_color.x, clear_color.y, clear_color.z, 1.0f });

			if (ImGui::Button("+"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			if (ImGui::Button("-"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter--;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}
	}

	void OnMessage(const Message& message) override
	{
		printf("%s: OnMessage called\n", name.c_str());
		std::cout << message.ToString() << std::endl;
	}

private:
	// Our state
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	int counter = 1;
	bool show_demo_window = false;
	bool show_another_window = false;
};

class Sandbox : public Application
{
public:
	Sandbox(HINSTANCE hInstance) : Application(hInstance)
	{
		layers.PushOverlay(new ExampleOverlay("1st Example Overlay"));
		layers.Push(new ExampleLayer());
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
