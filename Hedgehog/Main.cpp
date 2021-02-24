
// TODO: Applications using the Hedgehog engine should just include some Hedgehog.h header
//       and then create a concrete application class inheriting from the Hedgehog Application class
//       altough so far everything needed is in the Application anyway
// TODECIDE: Should the main function/entry point be a part of the engine or should it be up to the application (as it is here)?
#include <Application/Application.h>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
//#include <spdlog/spdlog.h>
//#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

#include <imgui_internal.h>


void loadModel(std::string& filename, long long int& numberOfVertices, float*& vertices, long long int& numberOfIndices, unsigned int*& indices)
{
	std::ifstream in(filename);
	char buffer[256];

	// header, just discard
	in.getline(buffer, 256);

	// number of vertices
	std::string text;
	in >> text >> numberOfVertices;
	in >> text >> numberOfIndices;

	vertices = new float[numberOfVertices * 3 * 2];
	indices = new unsigned int[numberOfIndices * 3];

	for (int vertex = 0; vertex < numberOfVertices; vertex++)
	{
		in >> vertices[vertex * 6 + 0] >> vertices[vertex * 6 + 1] >> vertices[vertex * 6 + 2];
		vertices[vertex * 6 + 3] = vertices[vertex * 6 + 4] = vertices[vertex * 6 + 5] = 0.0;
	}

	for (int index = 0; index < numberOfIndices; index++)
	{
		in >> indices[index * 3 + 0] >> indices[index * 3 + 1] >> indices[index * 3 + 2];

		// get the triangle normal
		glm::vec3 v0(vertices[indices[index * 3 + 0] * 6 + 0], vertices[indices[index * 3 + 0] * 6 + 1], vertices[indices[index * 3 + 0] * 6 + 2]);
		glm::vec3 v1(vertices[indices[index * 3 + 1] * 6 + 0], vertices[indices[index * 3 + 1] * 6 + 1], vertices[indices[index * 3 + 1] * 6 + 2]);
		glm::vec3 v2(vertices[indices[index * 3 + 2] * 6 + 0], vertices[indices[index * 3 + 2] * 6 + 1], vertices[indices[index * 3 + 2] * 6 + 2]);

		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

		vertices[indices[index * 3 + 0] * 6 + 3] += normal.x;
		vertices[indices[index * 3 + 0] * 6 + 4] += normal.y;
		vertices[indices[index * 3 + 0] * 6 + 5] += normal.z;

		vertices[indices[index * 3 + 1] * 6 + 3] += normal.x;
		vertices[indices[index * 3 + 1] * 6 + 4] += normal.y;
		vertices[indices[index * 3 + 1] * 6 + 5] += normal.z;

		vertices[indices[index * 3 + 2] * 6 + 3] += normal.x;
		vertices[indices[index * 3 + 2] * 6 + 4] += normal.y;
		vertices[indices[index * 3 + 2] * 6 + 5] += normal.z;
	}
}

class ExampleLayer : public Layer
{
public:
	ExampleLayer(bool enable = true) :
		Layer("Example Layer", enable),
		wireframeMode(RenderCommand::GetWireframeMode()),
		depthTest(RenderCommand::GetDepthTest()),
		faceCulling(RenderCommand::GetFaceCulling()),
		blending(RenderCommand::GetBlending())
	{
		aspectRatio = (float)Application::GetInstance().GetWindow().GetWidth() / (float)Application::GetInstance().GetWindow().GetHeight();
		camera = PerspectiveCamera(56.0f, aspectRatio, 0.01f, 25.0f); // camera space, +z goes into the screen
		//camera = OrthographicCamera(-aspectRatio, aspectRatio, -1.0f, 1.0f, 0.01f, 25.0f)

		camera.SetPosition({ 1.0f, 1.0f, 3.0f }); // world space, +z goes out of the screen
		camera.SetRotation({ -10.0f, 20.0f, 0.0f });

		long long int numberOfVertices;
		long long int numberOfIndices;
		float* modelVertices = NULL;
		unsigned int* modelIndices = NULL;
		std::string modelFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Model\\bunny.tri";
		loadModel(modelFilename, numberOfVertices, modelVertices, numberOfIndices, modelIndices);

		BufferLayout modelVertexBufferArrayLayout =
		{
			{ ShaderDataType::Float3, "a_position" },
			{ ShaderDataType::Float3, "a_normal" },
		};

		ConstantBufferDescription modelconstBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4) },
			{ "u_Transform", sizeof(glm::mat4) },
		};

		std::string modelVertexSrc;
		std::string modelFragmentSrc;
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			modelVertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLModelVertexShader.glsl";
			modelFragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLModelPixelShader.glsl";
		}
		else if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
		{
			modelVertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ModelShader.hlsl";
			modelFragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ModelShader.hlsl";
		}
		modelShader.reset(Shader::Create(modelVertexSrc, modelFragmentSrc));
		modelShader->SetupConstantBuffers(modelconstBufferDesc);
		
		modelVertexArray.reset(VertexArray::Create(modelShader, modelVertexBufferArrayLayout));
		modelVertexBuffer.reset(VertexBuffer::Create(modelVertexBufferArrayLayout, modelVertices, sizeof(float) * 6 * (int)numberOfVertices));
		modelVertexArray->AddVertexBuffer(modelVertexBuffer);
		modelIndexBuffer.reset(IndexBuffer::Create(modelIndices, 3 * (int)numberOfIndices));
		modelVertexArray->AddIndexBuffer(modelIndexBuffer);

		ConstantBufferDescription constBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4) },
			{ "u_Transform", sizeof(glm::mat4) },
		};
		
		BufferLayout vertexBufferLayout =
		{
			{ ShaderDataType::Float4, "a_position" },
			{ ShaderDataType::Float4, "a_color" },
			{ ShaderDataType::Float2, "a_textureCoordinates"}
		};

		float vertices[] =
		{
			// 1x1x1 cube centered around the origin (0, 0, 0)
			// front face - white with some red on the bottom
			-0.5f,  0.5f,  0.5f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f, // top left
			 0.5f,  0.5f,  0.5f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f, // top right
			-0.5f, -0.5f,  0.5f, 1.0f,		1.0f, 0.8f, 0.8f, 1.0f,		0.0f, 0.0f, // bottom left
			 0.5f, -0.5f,  0.5f, 1.0f,		1.0f, 0.8f, 0.8f, 1.0f,		1.0f, 0.0f, // bottom right
			 // back face - black with some red on the bottom
			-0.5f,  0.5f, -0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, // top left
			 0.5f,  0.5f, -0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f, // top right
			-0.5f, -0.5f, -0.5f, 1.0f,		0.2f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, // bottom left
			 0.5f, -0.5f, -0.5f, 1.0f,		0.2f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, // bottom right
		};

		// Shaders
		std::string vertexSrc;
		std::string fragmentSrc;
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			vertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLExampleVertexShader.glsl";
			fragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLExamplePixelShader.glsl";
		}
		else if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
		{
			vertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
			fragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
		}
		shader.reset(Shader::Create(vertexSrc, fragmentSrc));
		shader->SetupConstantBuffers(constBufferDesc);

		std::string vertexSrcTexture;
		std::string fragmentSrcTexture;
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			vertexSrcTexture = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLTextureVertexShader.glsl";
			fragmentSrcTexture = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLTexturePixelShader.glsl";
		}
		else if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
		{
			vertexSrcTexture = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12TextureShader.hlsl";
			fragmentSrcTexture = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12TextureShader.hlsl";
		}
		textureShader.reset(Shader::Create(vertexSrcTexture, fragmentSrcTexture));
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL)
		{
			textureShader->UploadConstant("u_texture", 0);
		}
		textureShader->SetupConstantBuffers(constBufferDesc);

		// Textures
		std::string textureFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Texture\\ezi.png";
		texture.reset(Texture2D::Create(textureFilename));

		// Vertex Arrays
		vertexArray.reset(VertexArray::Create(shader, vertexBufferLayout));

		vertexArraySquare.reset(VertexArray::Create(textureShader, vertexBufferLayout, texture));

		// Vertex Buffers
		vertexBuffer.reset(VertexBuffer::Create(vertexBufferLayout, vertices, sizeof(vertices)));
		vertexArray->AddVertexBuffer(vertexBuffer);

		vertexBufferSquare.reset(VertexBuffer::Create(vertexBufferLayout, vertices, sizeof(vertices) / 2));
		vertexArraySquare->AddVertexBuffer(vertexBufferSquare);

		// Index Buffers
		unsigned int indices[] = { 0,2,1, 1,2,3, 4,5,7, 4,7,6, 2,6,3, 3,6,7, 0,5,4, 0,1,5, 1,3,7, 1,7,5, 0,4,2, 2,4,6 };
		indexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(unsigned int)));
		vertexArray->AddIndexBuffer(indexBuffer);

		unsigned int indicesSquare[] = { 0,2,1, 1,2,3 };
		indexBufferSquare.reset(IndexBuffer::Create(indicesSquare, sizeof(indicesSquare) / sizeof(unsigned int)));
		vertexArraySquare->AddIndexBuffer(indexBufferSquare);

		// Transforms
		transform2 = glm::mat4x4(1.0f);
		transform2 = glm::translate(transform2, glm::vec3(3.0f, 0.25f, 0.5f));
		transform2 = glm::rotate(transform2, glm::radians(-20.0f), glm::vec3(0.0, 1.0, 0.0));
		transform2 = glm::rotate(transform2, glm::radians(180.0f), glm::vec3(0.0, 0.0, 1.0));
		transform2 = glm::scale(transform2, glm::vec3(1.5f, 1.5f, 1.5f));

		transform3 = glm::mat4x4(1.0f);
		transform3 = glm::translate(transform3, glm::vec3(1.5f, 2.0f, -0.5f));
		transform3 = glm::rotate(transform3, glm::radians(-10.0f), glm::vec3(0.0, 1.0, 0.0));
		transform3 = glm::rotate(transform3, glm::radians(45.0f), glm::vec3(0.0, 0.0, 1.0));
		transform3 = glm::scale(transform3, glm::vec3(0.5f, 1.0f, 0.5f));
	}

	void OnUpdate(const std::chrono::duration<double, std::milli>& duration) override
	{
		Renderer::SetWireframeMode(wireframeMode);
		Renderer::SetDepthTest(depthTest);
		Renderer::SetFaceCulling(faceCulling);
		Renderer::SetBlending(blending);

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
		}

		if (GetKeyState(0x6D) < 0) // '-'
		{
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


		Renderer::BeginScene(camera);
		{
			//if (showAxes)
			//{
			//	BufferLayout axesBL =
			//	{
			//		{ ShaderDataType::Float4, "a_position" },
			//		{ ShaderDataType::Float4, "a_color" },
			//	};

			//	float axesVertices[] =
			//	{
			//		0.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f,
			//		10.0f, 0.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f, 1.0f,
			//		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,
			//		0.0f, 10.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f, 1.0f,
			//		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,
			//		0.0f, 0.0f, 10.0f, 1.0f,	0.0f, 0.0f, 1.0f, 1.0f,
			//	};

			//	unsigned int axesIndices[] = { 0, 1, 2, 3, 4, 5 };

			//	VertexBuffer* axesVB = VertexBuffer::Create(axesBL, axesVertices, sizeof(axesVertices));
			//	glEnableVertexAttribArray(0);
			//	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, 0);
			//	glEnableVertexAttribArray(1);
			//	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (const void*)16);

			//	IndexBuffer* axesIB = IndexBuffer::Create(axesIndices, sizeof(axesIndices) / sizeof(unsigned int));

			//	shader->Bind();
			//	shader->UploadConstant("u_ViewProjection", camera.GetProjectionView());
			//	shader->UploadConstant("u_Transform", glm::mat4x4(1.0f));
			//	glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, nullptr);
			//	shader->Unbind();

			//	delete axesVB;
			//	delete axesIB;
			//}

			Renderer::Submit(vertexArray, glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)));
			Renderer::Submit(vertexArray, transform3);
			Renderer::Submit(vertexArray, transform2);

			Renderer::Submit(modelVertexArray, glm::rotate(glm::mat4x4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::rotate(glm::mat4x4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

			// Order matters when we want to blend
			Renderer::Submit(vertexArraySquare, glm::translate(glm::mat4x4(1.0f), glm::vec3(0.0, 2.0f, 0.0f)));
		}
		Renderer::EndScene();
	}

	void OnGuiUpdate() override
	{
		ImGui::Begin("Window");
		ImGui::Text("Client Area Size: %u %u", Application::GetInstance().GetWindow().GetWidth(), Application::GetInstance().GetWindow().GetHeight());
		ImGui::End();

		ImGui::Begin("Camera");
		glm::vec3 position = camera.GetPosition();
		glm::vec3 rotation = camera.GetRotation();
		ImGui::Text("Position: %f %f %f", position.x, position.y, position.z);
		ImGui::Text("Rotation: %f %f %f", rotation.x, rotation.y, rotation.z);
		ImGui::End();

		ImGui::Begin("Rendering Settings");
		if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		ImGui::Checkbox("Wireframe Mode", &wireframeMode);
		ImGui::Checkbox("Depth Test", &depthTest);
		ImGui::Checkbox("Face Culling", &faceCulling);
		ImGui::Checkbox("Blending", &blending);
		if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		ImGui::End();

		ImGui::Begin("Scene");
		ImGui::Checkbox("Show Axes", &showAxes);
		ImGui::End();
	}

	void OnMessage(const Message& message) override
	{
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
				// Rotate camera only if the right mouse button is pressed
				if (GetKeyState(VK_RBUTTON) < 0)
				{
					yRotation -= ((float)mouseMoveMessage.GetX() - (float)lastX);
					xRotation -= ((float)mouseMoveMessage.GetY() - (float)lastY);
				}

				lastX = mouseMoveMessage.GetX();
				lastY = mouseMoveMessage.GetY();
			}
		}

		if (message.GetMessageType() == MessageType::WindowSize)
		{
			const WindowSizeMessage& windowSizeMessage = dynamic_cast<const WindowSizeMessage&>(message);

			aspectRatio = (float)windowSizeMessage.GetWidth() / (float)windowSizeMessage.GetHeight();
			camera.SetAspectRatio(aspectRatio);
		}
	}

private:
	float aspectRatio;
	PerspectiveCamera camera;
	//OrthographicCamera camera;

	bool wireframeMode;
	bool depthTest;
	bool faceCulling;
	bool blending;

	bool showAxes = true;

	std::shared_ptr<VertexArray> vertexArray;
	std::shared_ptr<VertexArray> vertexArraySquare;
	std::shared_ptr<VertexBuffer> vertexBuffer;
	std::shared_ptr<VertexBuffer> vertexBufferSquare;
	std::shared_ptr<IndexBuffer> indexBuffer;
	std::shared_ptr<IndexBuffer> indexBufferSquare;

	std::shared_ptr<Shader> shader;
	std::shared_ptr<Shader> textureShader;

	std::shared_ptr<Texture> texture;

	std::shared_ptr<VertexArray> modelVertexArray;
	std::shared_ptr<VertexBuffer> modelVertexBuffer;
	std::shared_ptr<IndexBuffer> modelIndexBuffer;
	std::shared_ptr<Shader> modelShader;

	glm::mat4x4 transform2;
	glm::mat4x4 transform3;

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
