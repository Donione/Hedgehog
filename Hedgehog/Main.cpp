
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

#include <Renderer/DirectX12VertexArray.h>
#include <glm/gtc/type_ptr.hpp>

#include <Component/Transform.h>
#include <Component/Light.h>


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

class ExampleLayer : public Hedge::Layer
{
public:
	ExampleLayer(bool enable = true) :
		Layer("Example Layer", enable)
	{
		previousWireframeMode = wireframeMode = Hedge::RenderCommand::GetWireframeMode();
		previousDepthTest = depthTest = Hedge::RenderCommand::GetDepthTest();
		previousFaceCulling = faceCulling = Hedge::RenderCommand::GetFaceCulling();
		previousBlending = blending = Hedge::RenderCommand::GetBlending();

		aspectRatio = (float)Hedge::Application::GetInstance().GetWindow().GetWidth() / (float)Hedge::Application::GetInstance().GetWindow().GetHeight();
		perspectiveCamera = Hedge::PerspectiveCamera(cameraFOV, aspectRatio, 0.01f, 25.0f); // camera space, +z goes into the screen
		orthographicCamera = Hedge::OrthographicCamera(-aspectRatio, aspectRatio, -1.0f, 1.0f, 0.01f, 25.0f);
		orthographicCamera.SetZoom(cameraZoom);
		camera = &perspectiveCamera;

		orthographicCamera.SetPosition(glm::vec3(1.0f, 1.0f, 3.0f)); // world space, +z goes out of the screen
		orthographicCamera.SetRotation(glm::vec3(-10.0f, 20.0f, 0.0f));
		perspectiveCamera.SetPosition(glm::vec3(1.0f, 1.0f, 3.0f)); // world space, +z goes out of the screen
		perspectiveCamera.SetRotation(glm::vec3(-10.0f, 20.0f, 0.0f));

		light.SetColor(glm::vec3(1.0f, 1.0f, 1.0f));
		light.SetPosition(glm::vec3(3.0f, 3.0f, 5.0f));

		long long int numberOfVertices;
		long long int numberOfIndices;
		float* modelVertices = NULL;
		unsigned int* modelIndices = NULL;
		std::string modelFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Model\\bunny.tri";
		loadModel(modelFilename, numberOfVertices, modelVertices, numberOfIndices, modelIndices);

		Hedge::BufferLayout modelVertexBufferArrayLayout =
		{
			{ Hedge::ShaderDataType::Float3, "a_position" },
			{ Hedge::ShaderDataType::Float3, "a_normal" },
		};

		Hedge::ConstantBufferDescription modelconstBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_viewPos", sizeof(glm::vec3), Hedge::ConstantBufferUsage::Scene, },
			{ "u_lightPosition", sizeof(glm::vec3), Hedge::ConstantBufferUsage::Scene, },
			{ "u_lightColor", sizeof(glm::vec3), Hedge::ConstantBufferUsage::Scene, },
		};

		std::string modelVertexSrc;
		std::string modelFragmentSrc;
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			modelVertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLModelVertexShader.glsl";
			modelFragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLModelPixelShader.glsl";
		}
		else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			modelVertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ModelShader.hlsl";
			modelFragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ModelShader.hlsl";
		}
		modelShader.reset(Hedge::Shader::Create(modelVertexSrc, modelFragmentSrc));
		modelShader->SetupConstantBuffers(modelconstBufferDesc);

		modelVertexArray.reset(Hedge::VertexArray::Create(modelShader, modelVertexBufferArrayLayout));
		modelVertexBuffer.reset(Hedge::VertexBuffer::Create(modelVertexBufferArrayLayout, modelVertices, sizeof(float) * 6 * (int)numberOfVertices));
		delete modelVertices;
		modelVertexArray->AddVertexBuffer(modelVertexBuffer);
		modelIndexBuffer.reset(Hedge::IndexBuffer::Create(modelIndices, 3 * (int)numberOfIndices));
		delete modelIndices;
		modelVertexArray->AddIndexBuffer(modelIndexBuffer);
		modelTransform.SetTranslation(translation);
		modelTransform.SetRotation(rotate);
		modelTransform.SetUniformScale(scale);


		Hedge::ConstantBufferDescription constBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
		};
		
		Hedge::BufferLayout vertexBufferLayout =
		{
			{ Hedge::ShaderDataType::Float4, "a_position" },
			{ Hedge::ShaderDataType::Float4, "a_color" },
			{ Hedge::ShaderDataType::Float2, "a_textureCoordinates"}
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
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			vertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLExampleVertexShader.glsl";
			fragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLExamplePixelShader.glsl";
		}
		else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			vertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
			fragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
		}
		shader.reset(Hedge::Shader::Create(vertexSrc, fragmentSrc));
		shader->SetupConstantBuffers(constBufferDesc);

		std::string vertexSrcTexture;
		std::string fragmentSrcTexture;
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			vertexSrcTexture = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLTextureVertexShader.glsl";
			fragmentSrcTexture = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLTexturePixelShader.glsl";
		}
		else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			vertexSrcTexture = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12TextureShader.hlsl";
			fragmentSrcTexture = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12TextureShader.hlsl";
		}
		textureShader.reset(Hedge::Shader::Create(vertexSrcTexture, fragmentSrcTexture));
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			textureShader->UploadConstant("u_texture", 0);
		}
		textureShader->SetupConstantBuffers(constBufferDesc);

		// Textures
		std::string textureFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Texture\\ChernoLogo.png";
		texture.reset(Hedge::Texture2D::Create(textureFilename));

		// Vertex Arrays
		vertexArray.reset(Hedge::VertexArray::Create(shader, vertexBufferLayout));

		vertexArraySquare.reset(Hedge::VertexArray::Create(textureShader, vertexBufferLayout, texture));

		// Vertex Buffers
		vertexBuffer.reset(Hedge::VertexBuffer::Create(vertexBufferLayout, vertices, sizeof(vertices)));
		vertexArray->AddVertexBuffer(vertexBuffer);

		vertexBufferSquare.reset(Hedge::VertexBuffer::Create(vertexBufferLayout, vertices, sizeof(vertices) / 2));
		vertexArraySquare->AddVertexBuffer(vertexBufferSquare);

		// Index Buffers
		unsigned int indices[] = { 0,2,1, 1,2,3, 4,5,7, 4,7,6, 2,6,3, 3,6,7, 0,5,4, 0,1,5, 1,3,7, 1,7,5, 0,4,2, 2,4,6 };
		indexBuffer.reset(Hedge::IndexBuffer::Create(indices, sizeof(indices) / sizeof(unsigned int)));
		vertexArray->AddIndexBuffer(indexBuffer);

		unsigned int indicesSquare[] = { 0,2,1, 1,2,3 };
		indexBufferSquare.reset(Hedge::IndexBuffer::Create(indicesSquare, sizeof(indicesSquare) / sizeof(unsigned int)));
		vertexArraySquare->AddIndexBuffer(indexBufferSquare);

		// Transforms
		transform2.Translate(glm::vec3(3.0f, 0.25f, 0.5f));
		transform2.Rotate(glm::vec3(0.0f, -20.0f, 180.0f));
		transform2.UniformScale(1.5f);

		transform3.SetTranslation(glm::vec3(1.5f, 2.0f, -0.5f));
		transform3.SetRotation(glm::vec3(0.0f, -10.0f, 45.0f));
		transform3.SetScale(glm::vec3(0.5f, 1.0f, 0.5f));



		float* lightVertices = NULL;
		unsigned int* lightIndices = NULL;

		modelFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Model\\koule.tri";
		loadModel(modelFilename, numberOfVertices, lightVertices, numberOfIndices, lightIndices);

		Hedge::BufferLayout lightVertexBufferArrayLayout =
		{
			{ Hedge::ShaderDataType::Float3, "a_position" },
			{ Hedge::ShaderDataType::Float3, "a_normal" },
		};

		Hedge::ConstantBufferDescription lightconstBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_lightColor", sizeof(glm::vec3), Hedge::ConstantBufferUsage::Scene, },
		};

		std::string lightVertexSrc;
		std::string lightFragmentSrc;
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			lightVertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ModelExampleShader.hlsl";
			lightFragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\DirectX12ModelExampleShader.hlsl";
		}
		else
		{
			lightVertexSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLModelExampleVertexShader.glsl";
			lightFragmentSrc = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Shader\\OpenGLModelExamplePixelShader.glsl";
		}
		lightShader.reset(Hedge::Shader::Create(lightVertexSrc, lightFragmentSrc));
		lightShader->SetupConstantBuffers(lightconstBufferDesc);

		lightVertexArray.reset(Hedge::VertexArray::Create(lightShader, lightVertexBufferArrayLayout));
		lightVertexBuffer.reset(Hedge::VertexBuffer::Create(lightVertexBufferArrayLayout, lightVertices, sizeof(float) * 6 * (int)numberOfVertices));
		delete lightVertices;
		lightVertexArray->AddVertexBuffer(lightVertexBuffer);
		lightIndexBuffer.reset(Hedge::IndexBuffer::Create(lightIndices, 3 * (int)numberOfIndices));
		delete lightIndices;
		lightVertexArray->AddIndexBuffer(lightIndexBuffer);
		light.SetPosition(glm::vec3(lightX, lightY, lightZ));
		light.GetTransform().SetUniformScale(0.1f);
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
			//camera.SetPosition({ 0.0f, 0.0f, 3.0f });
			orthographicCamera.SetPosition({ 0.0f, 0.0f, 0.0f });
			orthographicCamera.SetRotation({ 0.0f, 0.0f, 0.0f });
			perspectiveCamera.SetPosition({ 0.0f, 0.0f, 0.0f });
			perspectiveCamera.SetRotation({ 0.0f, 0.0f, 0.0f });
		}

		orthographicCamera.Move(glm::vec3(xOffset * movementSpeed * (float)duration.count(), yOffset * scrollSpeed, zOffset * movementSpeed * (float)duration.count()));
		orthographicCamera.Rotate(glm::vec3(mouseSpeed * xRotation, mouseSpeed * yRotation, zRotation * rotationSpeed * (float)duration.count()));
		perspectiveCamera.Move(glm::vec3(xOffset * movementSpeed * (float)duration.count(), yOffset * scrollSpeed, zOffset * movementSpeed * (float)duration.count()));
		perspectiveCamera.Rotate(glm::vec3(mouseSpeed * xRotation, mouseSpeed * yRotation, zRotation * rotationSpeed * (float)duration.count()));

		xOffset = 0;
		yOffset = 0;
		zOffset = 0;

		xRotation = 0;
		yRotation = 0;
		zRotation = 0;


		Hedge::Renderer::BeginScene(camera);
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

			Hedge::Renderer::Submit(vertexArray, glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f)));
			Hedge::Renderer::Submit(vertexArray, transform2.Get());
			Hedge::Renderer::Submit(vertexArray, transform3.Get());
			
			lightVertexArray->GetShader()->UploadConstant("u_lightColor", light.GetColor());
			Hedge::Renderer::Submit(lightVertexArray, light.GetTransform().Get());
			
			modelVertexArray->GetShader()->UploadConstant("u_viewPos", camera->GetPosition());
			modelVertexArray->GetShader()->UploadConstant("u_lightColor", light.GetColor());
			modelVertexArray->GetShader()->UploadConstant("u_lightPosition", light.GetPosition());
			Hedge::Renderer::Submit(modelVertexArray, modelTransform.Get());

			// Order matters when we want to blend
			Hedge::Renderer::Submit(vertexArraySquare, glm::translate(glm::mat4x4(1.0f), glm::vec3(-1.0f, 2.0f, 0.0f)));
		}
		Hedge::Renderer::EndScene();

		// Update render settings at the end of the scene because current frame begun with the old settings
		// really, the settings should be done after the EndFrame but we have just one layer atm
		bool updatePSO = false;
		if (wireframeMode != previousWireframeMode)
		{
			previousWireframeMode = wireframeMode;
			Hedge::Renderer::SetWireframeMode(wireframeMode);
			updatePSO = true;
		}

		if (depthTest != previousDepthTest)
		{
			previousDepthTest = depthTest;
			Hedge::Renderer::SetDepthTest(depthTest);
			updatePSO = true;
		}

		if (faceCulling != previousFaceCulling)
		{
			previousFaceCulling = faceCulling;
			Hedge::Renderer::SetFaceCulling(faceCulling);
			updatePSO = true;
		}

		if (blending != previousBlending)
		{
			previousBlending = blending;
			Hedge::Renderer::SetBlending(blending);
			updatePSO = true;
		}

		if (updatePSO)
		{
			// TODO obviously, this is just temporary
			if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
			{
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(vertexArray)->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(vertexArraySquare)->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(modelVertexArray)->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(lightVertexArray)->UpdateRenderSettings();
			}
		}
	}

	void OnGuiUpdate() override
	{
		ImGui::Begin("Window");
		ImGui::Text("Client Area Size: %u %u", Hedge::Application::GetInstance().GetWindow().GetWidth(), Hedge::Application::GetInstance().GetWindow().GetHeight());
		ImGui::End();


		ImGui::Begin("Camera");

		static int cameraType = 1;
		ImGui::Combo("Camera Type", &cameraType, "Orthographic\0Perspective\0\0");
		if (cameraType == 0)
		{
			camera = &orthographicCamera;
		}
		else
		{
			camera = &perspectiveCamera;
		}

		glm::vec3 position = camera->GetPosition();
		glm::vec3 rotation = camera->GetRotation();
		ImGui::Text("Position: %f %f %f", position.x, position.y, position.z);
		ImGui::Text("Rotation: %f %f %f", rotation.x, rotation.y, rotation.z);

		if (camera->GetType() == Hedge::CameraType::Orthographic)
		{
			ImGui::SliderFloat("Zoom", &cameraZoom, 0.1f, 10.0f);
			orthographicCamera.SetZoom(cameraZoom);
		}
		else if (camera->GetType() == Hedge::CameraType::Perspective)
		{
			ImGui::SliderFloat("vFOV", &cameraFOV, 20.0f, 150.0f);
			perspectiveCamera.SetFOV(cameraFOV);
			ImGui::SameLine();
			ImGui::Text("(hFOV: %f)", cameraFOV * aspectRatio);
		}
		ImGui::End();


		ImGui::Begin("Rendering Settings");
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			ImGui::Text("API used: OpenGL");
		}
		else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			ImGui::Text("API used: DirectX12");
		}

		// Following code snippet can be used to disable ImGui controls and grey them out
		//if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
		//{
		//	ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		//	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		//}
		//doStuff
		//if (Renderer::GetAPI() == RendererAPI::API::DirectX12)
		//{
		//	ImGui::PopItemFlag();
		//	ImGui::PopStyleVar();
		//}

		ImGui::Checkbox("Wireframe Mode", &wireframeMode);
		ImGui::Checkbox("Depth Test", &depthTest);
		ImGui::Checkbox("Face Culling", &faceCulling);
		ImGui::Checkbox("Blending", &blending);
		ImGui::End();


		ImGui::Begin("Scene");
		ImGui::Checkbox("Show Axes", &showAxes);

		ImGui::Text("\nModel transform:");
		ImGui::SliderFloat3("translate", glm::value_ptr(translation), -30.0f, 30.0f);
		ImGui::SliderFloat3("rotate", glm::value_ptr(rotate), -360.0, 360.0);
		ImGui::SliderFloat("scale", &scale, 0.01f, 10.0f);
		modelTransform.SetTranslation(translation);
		modelTransform.SetRotation(rotate);
		modelTransform.SetUniformScale(scale);

		ImGui::Text("\nLight properties:");
		ImGui::ColorEdit3("Color", glm::value_ptr(light.GetColor()));
		glm::vec3 lightPosition = light.GetPosition();
		ImGui::DragFloat3("Position", glm::value_ptr(lightPosition), 0.01f, -20.0f, 20.0f);

		light.SetPosition(glm::vec3(glm::sin(lightX) * 3.0f, glm::sin(lightY) * 3.0f, glm::cos(lightZ) * 3.0f));
		lightX += 0.001f;
		lightY += 0.0013f;
		lightZ += 0.001f;
		ImGui::End();
	}

	void OnMessage(const Hedge::Message& message) override
	{
		if (message.GetMessageType() == Hedge::MessageType::MouseScrolled)
		{
			const Hedge::MouseScrollMessage& mouseScrollMessage = dynamic_cast<const Hedge::MouseScrollMessage&>(message);

			yOffset += mouseScrollMessage.GetDistance();
		}

		if (message.GetMessageType() == Hedge::MessageType::MouseMoved)
		{
			const Hedge::MouseMoveMessage& mouseMoveMessage = dynamic_cast<const Hedge::MouseMoveMessage&>(message);

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

		if (message.GetMessageType() == Hedge::MessageType::WindowSize)
		{
			const Hedge::WindowSizeMessage& windowSizeMessage = dynamic_cast<const Hedge::WindowSizeMessage&>(message);

			aspectRatio = (float)windowSizeMessage.GetWidth() / (float)windowSizeMessage.GetHeight();
			orthographicCamera.SetAspectRatio(aspectRatio);
			perspectiveCamera.SetAspectRatio(aspectRatio);
		}
	}

private:
	float aspectRatio;
	float cameraFOV = 56.0f;
	float cameraZoom = 1.0f;
	Hedge::Camera* camera;
	Hedge::PerspectiveCamera perspectiveCamera;
	Hedge::OrthographicCamera orthographicCamera;

	bool wireframeMode;
	bool depthTest;
	bool faceCulling;
	bool blending;

	bool previousWireframeMode;
	bool previousDepthTest;
	bool previousFaceCulling;
	bool previousBlending;

	bool showAxes = true;

	std::shared_ptr<Hedge::VertexArray> vertexArray;
	std::shared_ptr<Hedge::VertexArray> vertexArraySquare;
	std::shared_ptr<Hedge::VertexBuffer> vertexBuffer;
	std::shared_ptr<Hedge::VertexBuffer> vertexBufferSquare;
	std::shared_ptr<Hedge::IndexBuffer> indexBuffer;
	std::shared_ptr<Hedge::IndexBuffer> indexBufferSquare;

	std::shared_ptr<Hedge::Shader> shader;
	std::shared_ptr<Hedge::Shader> textureShader;

	std::shared_ptr<Hedge::Texture> texture;

	std::shared_ptr<Hedge::VertexArray> modelVertexArray;
	std::shared_ptr<Hedge::VertexBuffer> modelVertexBuffer;
	std::shared_ptr<Hedge::IndexBuffer> modelIndexBuffer;
	std::shared_ptr<Hedge::Shader> modelShader;

	Hedge::Transform transform2;
	Hedge::Transform transform3;
	Hedge::Transform modelTransform;

	glm::vec3 translation = glm::vec3(0.0f);
	glm::vec3 rotate = glm::vec3(0.0f, 180.0f, 180.0f);
	float scale = 1.0f;


	std::shared_ptr<Hedge::VertexArray> lightVertexArray;
	std::shared_ptr<Hedge::VertexBuffer> lightVertexBuffer;
	std::shared_ptr<Hedge::IndexBuffer> lightIndexBuffer;
	std::shared_ptr<Hedge::Shader> lightShader;

	Hedge::Light light;
	float lightX = 0.0f;
	float lightY = 0.0f;
	float lightZ = 0.0f;

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

class ExampleOverlay : public Hedge::Layer
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
			Hedge::RenderCommand::SetClearColor({ clear_color.x, clear_color.y, clear_color.z, 1.0f });

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

	void OnMessage(const Hedge::Message& message) override
	{
	}

private:
	// Our state
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	int counter = 1;
	bool show_demo_window = false;
	bool show_another_window = false;
};

class Sandbox : public Hedge::Application
{
public:
	Sandbox(HINSTANCE hInstance) : Application(hInstance)
	{
		layers.PushOverlay(new ExampleOverlay("1st Example Overlay"));
		layers.Push(new ExampleLayer());
	}

	~Sandbox()
	{
		Hedge::Layer* layer;
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
