
// TODO: Applications using the Hedgehog engine should just include some Hedgehog.h header
//       and then create a concrete application class inheriting from the Hedgehog Application class
//       altough so far everything needed is in the Application anyway
// TODECIDE: Should the main function/entry point be a part of the engine or should it be up to the application (as it is here)?
#include <Application/Application.h>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
//#include <spdlog/spdlog.h>
//#include "spdlog/sinks/stdout_color_sinks.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>

#include <imgui_internal.h>

#include <Renderer/DirectX12VertexArray.h>
#include <glm/gtc/type_ptr.hpp>

#include <Component/Transform.h>
#include <Component/Light.h>
#include <Component/Mesh.h>


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
		perspectiveCamera = Hedge::PerspectiveCamera(cameraFOV, aspectRatio, 0.01f, 100.0f); // camera space, +z goes into the screen
		orthographicCamera = Hedge::OrthographicCamera(-aspectRatio, aspectRatio, -1.0f, 1.0f, 0.01f, 25.0f);
		orthographicCamera.SetZoom(cameraZoom);
		camera = &perspectiveCamera;

		orthographicCamera.SetPosition(glm::vec3(1.0f, 1.0f, 3.0f)); // world space, +z goes out of the screen
		orthographicCamera.SetRotation(glm::vec3(-10.0f, 20.0f, 0.0f));
		perspectiveCamera.SetPosition(glm::vec3(1.0f, 1.0f, 3.0f)); // world space, +z goes out of the screen
		perspectiveCamera.SetRotation(glm::vec3(-10.0f, 20.0f, 0.0f));


		std::string modelFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Model\\bunny.tri";

		auto modelPrimitiveTopology = Hedge::PrimitiveTopology::Trinagle;
		Hedge::BufferLayout modelVertexBufferArrayLayout =
		{
			{ Hedge::ShaderDataType::Float3, "a_position" },
			{ Hedge::ShaderDataType::Float3, "a_normal" },
		};

		Hedge::ConstantBufferDescription modelconstBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_viewPos", sizeof(glm::vec3), Hedge::ConstantBufferUsage::Scene },
			{ "u_directionalLight", sizeof(Hedge::DirectionalLight), Hedge::ConstantBufferUsage::Light },
			{ "u_pointLight", sizeof(Hedge::PointLight), Hedge::ConstantBufferUsage::Light, 2 },
			{ "u_spotLight", sizeof(Hedge::SpotLight), Hedge::ConstantBufferUsage::Light },
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

		modelMesh = Hedge::Mesh(modelFilename, modelPrimitiveTopology, modelVertexBufferArrayLayout,
								modelVertexSrc, modelFragmentSrc, modelconstBufferDesc);

		modelMesh.transform.SetTranslation(translation);
		modelMesh.transform.SetRotation(rotate);
		modelMesh.transform.SetUniformScale(scale);


		Hedge::ConstantBufferDescription constBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
		};
		
		auto PrimitiveTopology = Hedge::PrimitiveTopology::Trinagle;

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

		// Textures
		std::string textureFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Texture\\ChernoLogo.png";

		unsigned int indices[] = { 0,2,1, 1,2,3, 4,5,7, 4,7,6, 2,6,3, 3,6,7, 0,5,4, 0,1,5, 1,3,7, 1,7,5, 0,4,2, 2,4,6 };
		unsigned int indicesSquare[] = { 0,2,1, 1,2,3 };


		mesh = Hedge::Mesh(vertices, sizeof(vertices),
						   indices, sizeof(indices) / sizeof(unsigned int),
						   PrimitiveTopology, vertexBufferLayout,
						   vertexSrc, fragmentSrc, constBufferDesc);
		mesh.transform.SetTranslation(glm::vec3(-2.0f, 0.0f, 0.0f));

		// Transforms
		transform2.Translate(glm::vec3(3.0f, 0.25f, 0.5f));
		transform2.Rotate(glm::vec3(0.0f, -20.0f, 180.0f));
		transform2.UniformScale(1.5f);

		transform3.SetTranslation(glm::vec3(1.5f, 2.0f, -0.5f));
		transform3.SetRotation(glm::vec3(0.0f, -10.0f, 45.0f));
		transform3.SetScale(glm::vec3(0.5f, 1.0f, 0.5f));

		squareMesh = Hedge::Mesh(vertices, sizeof(vertices) / 2,
								 indicesSquare, sizeof(indicesSquare) / sizeof(unsigned int),
								 PrimitiveTopology, vertexBufferLayout,
								 vertexSrcTexture, fragmentSrcTexture, constBufferDesc,
								 textureFilename);
		squareMesh.transform.SetTranslation(glm::vec3(-1.0f, 2.0f, 0.0f));

		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			squareMesh.GetShader()->UploadConstant("u_texture", 0);
		}


		// Lights
		// Directional Lights
		directionalLight.color = glm::vec3(1.0f, 0.8f, 0.0f);
		directionalLight.direction = glm::vec3(0.0f, 0.0f, -1.0f);

		// Pointlights
		pointLight[0].color = (glm::vec3(1.0f, 1.0f, 1.0f));
		pointLight[0].attenuation = (glm::vec3(1.0f, 0.027f, 0.0028f));
		pointLight[0].position = (glm::vec3(0.0f, 0.0f, 1.0f));

		pointLight[1].color = (glm::vec3(0.0f, 1.0f, 0.0f));
		pointLight[1].attenuation = (glm::vec3(1.0f, 0.027f, 0.0028f));
		pointLight[1].position = (glm::vec3(0.0f, 2.0f, 0.0f));

		modelFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Model\\koule.tri";

		auto lightPrimitiveTopology = Hedge::PrimitiveTopology::Trinagle;

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

		pointLightMeshes[0] = Hedge::Mesh(modelFilename, lightPrimitiveTopology, lightVertexBufferArrayLayout,
										  lightVertexSrc, lightFragmentSrc, lightconstBufferDesc);
		pointLightMeshes[0].transform.SetUniformScale(0.1f);


		pointLightMeshes[1] = Hedge::Mesh(modelFilename, lightPrimitiveTopology, lightVertexBufferArrayLayout,
										  lightVertexSrc, lightFragmentSrc, lightconstBufferDesc);
		pointLightMeshes[1].transform.SetUniformScale(0.1f);


		// Spotlight
		spotLight.color = (glm::vec3(1.0f, 1.0f, 1.0f));
		//spotLight.SetAttenuation(glm::vec3(1.0f, 0.027f, 0.0028f));
		spotLight.position = (glm::vec3(0.0f, 0.0f, 2.0f));
		spotLight.direction = (glm::vec3(0.0f, 0.0f, -1.0f));

		modelFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Model\\valec.tri";

		spotLightMesh = Hedge::Mesh(modelFilename, lightPrimitiveTopology, lightVertexBufferArrayLayout,
									lightVertexSrc, lightFragmentSrc, lightconstBufferDesc);
		spotLightMesh.transform.SetUniformScale(0.1f);
		spotLightMesh.transform.SetTranslation(glm::vec3(0.0f));
		spotLightMesh.transform.SetRotation(glm::vec3(0.0f));

		spotLightBaseRotation = glm::mat4(1.0f);
		spotLightBaseRotation = glm::rotate(spotLightBaseRotation, glm::radians(9.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		spotLightBaseRotation = glm::rotate(spotLightBaseRotation, glm::radians(-13.75f), glm::vec3(0.0f, 1.0f, 0.0f));


		// Axes lines
		auto axesPrimitiveTopology = Hedge::PrimitiveTopology::Line;

		Hedge::BufferLayout axesBL =
		{
			{ Hedge::ShaderDataType::Float4, "a_position" },
			{ Hedge::ShaderDataType::Float4, "a_color" },
			{ Hedge::ShaderDataType::Float2, "a_textureCoordinates"}
		};

		struct Vertex
		{
			float x, y, z, w, r, g, b, a, u, v;
		};

		Vertex axesVertices[6] =
		{
			{ -100.0f, 0.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f }, // X axis
			{ 100.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f },
			{ 0.0f, -100.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f }, // Y axis
			{ 0.0f, 100.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f },
			{ 0.0f, 0.0f, -100.0f, 1.0f,	0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f }, // Z axis
			{ 0.0f, 0.0f, 100.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f },
		};

		unsigned int axesIndices[] = { 0, 1, 2, 3, 4, 5 };

		const int numVertices = 804;
		Vertex gridVertices[numVertices];
		int index = 0;

		for (int x = -100; x <= 100; x++)
		{
			gridVertices[index++] = { (float)x, 0.0f, -100.0f, 1.0f,	0.3f, 0.3f, 0.3f, 1.0f,		0.0f, 0.0f };
			gridVertices[index++] = { (float)x, 0.0f,  100.0f, 1.0f,	0.3f, 0.3f, 0.3f, 1.0f,		0.0f, 0.0f };
		}

		for (int z = -100; z <= 100; z++)
		{
			gridVertices[index++] = { -100.0f, 0.0f, (float)z, 1.0f,	0.3f, 0.3f, 0.3f, 1.0f,		0.0f, 0.0f };
			gridVertices[index++] = {  100.0f, 0.0f, (float)z, 1.0f,	0.3f, 0.3f, 0.3f, 1.0f,		0.0f, 0.0f };
		}

		assert(index == numVertices);

		unsigned int gridIndices[numVertices];
		for (int i = 0; i < numVertices; i++)
		{
			gridIndices[i] = i;
		}


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

		constBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
		};

		axesMesh = Hedge::Mesh(&axesVertices[0].x, sizeof(axesVertices),
							   axesIndices, sizeof(axesIndices) / sizeof(unsigned int),
							   axesPrimitiveTopology, axesBL,
							   vertexSrc, fragmentSrc, constBufferDesc);

		gridMesh = Hedge::Mesh(&gridVertices[0].x, sizeof(gridVertices),
							   gridIndices, sizeof(gridIndices) / sizeof(unsigned int),
							   axesPrimitiveTopology, axesBL,
							   vertexSrc, fragmentSrc, constBufferDesc);
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
			if (showGrid)
			{
				Hedge::Renderer::Submit(gridMesh.Get());
			}

			if (showAxes)
			{
				Hedge::Renderer::Submit(axesMesh.Get());
			}

			//Hedge::Renderer::Submit(mesh.Get(), mesh.transform.Get());
			//Hedge::Renderer::Submit(mesh.Get(), transform2.Get());
			//Hedge::Renderer::Submit(mesh.Get(), transform3.Get());
			
			pointLightMeshes[0].GetShader()->UploadConstant("u_lightColor", pointLight[0].color);
			Hedge::Renderer::Submit(pointLightMeshes[0].Get(), pointLightMeshes[0].transform.Get());
			
			pointLightMeshes[1].GetShader()->UploadConstant("u_lightColor", pointLight[1].color);
			Hedge::Renderer::Submit(pointLightMeshes[1].Get(), pointLightMeshes[1].transform.Get());
			
			spotLightMesh.GetShader()->UploadConstant("u_lightColor", spotLight.color);
			Hedge::Renderer::Submit(spotLightMesh.Get(), spotLightMesh.transform.Get());
			
			modelMesh.GetShader()->UploadConstant("u_viewPos", camera->GetPosition());
			modelMesh.GetShader()->UploadConstant("u_directionalLight", directionalLight);
			modelMesh.GetShader()->UploadConstant("u_pointLight", pointLight, 2);
			modelMesh.GetShader()->UploadConstant("u_spotLight", spotLight);
			Hedge::Renderer::Submit(modelMesh.Get(), modelMesh.transform.Get());
			
			// Order matters when we want to blend
			//Hedge::Renderer::Submit(squareMesh.Get(), squareMesh.transform.Get());
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
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(mesh.Get())->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(squareMesh.Get())->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(modelMesh.Get())->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(pointLightMeshes[0].Get())->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(pointLightMeshes[1].Get())->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(spotLightMesh.Get())->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(axesMesh.Get())->UpdateRenderSettings();
				std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(gridMesh.Get())->UpdateRenderSettings();
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
		ImGui::Checkbox("Show Grid", &showGrid);

		ImGui::Text("\nModel transform:");
		ImGui::SliderFloat3("translate", glm::value_ptr(translation), -30.0f, 30.0f);
		rotate = modelMesh.transform.GetRotation();
		ImGui::SliderFloat3("rotate", glm::value_ptr(rotate), -360.0, 360.0);
		ImGui::SliderFloat("scale", &scale, 0.01f, 10.0f);
		modelMesh.transform.SetTranslation(translation);
		modelMesh.transform.SetRotation(rotate);
		modelMesh.transform.SetUniformScale(scale);
		ImGui::End();


		ImGui::Begin("Lights");
		ImGui::Text("Directional light");
		ImGui::ColorEdit3("Color", glm::value_ptr(directionalLight.color));
		ImGui::DragFloat3("Direction", glm::value_ptr(directionalLight.direction), 0.01f, -10.0, 10.0f);

		ImGui::Text("\nPoint light");
		ImGui::ColorEdit3("Color##2", glm::value_ptr(pointLight[0].color));
		ImGui::DragFloat3("Position##2", glm::value_ptr(pointLight[0].position), 0.01f, -20.0f, 20.0f);
		pointLightMeshes[0].transform.SetTranslation(pointLight[0].position);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * (1.0f / 4.0f));
		ImGui::DragFloat("##2x", &pointLight[0].attenuation.x, 0.1f, 0.0f, 10.f); ImGui::SameLine();
		ImGui::DragFloat("##2y", &pointLight[0].attenuation.y, 0.01f, 0.0f, 1.f); ImGui::SameLine();
		ImGui::DragFloat("Attenuation##2z", &pointLight[0].attenuation.z, 0.001f, 0.0f, 0.1f);
		ImGui::PopItemWidth();

		ImGui::Text("\nPoint light 2");
		ImGui::ColorEdit3("Color##22", glm::value_ptr(pointLight[1].color));
		ImGui::DragFloat3("Position##22", glm::value_ptr(pointLight[1].position), 0.01f, -20.0f, 20.0f);
		pointLightMeshes[1].transform.SetTranslation(pointLight[1].position);
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * (1.0f / 4.0f));
		ImGui::DragFloat("##22x", &pointLight[1].attenuation.x, 0.1f, 0.0f, 10.f); ImGui::SameLine();
		ImGui::DragFloat("##22y", &pointLight[1].attenuation.y, 0.01f, 0.0f, 1.f); ImGui::SameLine();
		ImGui::DragFloat("Attenuation##22z", &pointLight[1].attenuation.z, 0.001f, 0.0f, 0.1f);
		ImGui::PopItemWidth();

		ImGui::Text("\nSpot light");
		ImGui::ColorEdit3("Color##3", glm::value_ptr(spotLight.color));
		ImGui::DragFloat3("Position##3", glm::value_ptr(spotLight.position), 0.01f, -20.0f, 20.0f);
		spotLightMesh.transform.SetTranslation(spotLight.position);
		ImGui::DragFloat3("Direction##3", glm::value_ptr(spotLight.direction), 0.01f, -1.0, 1.0f);
		auto dir2rot = glm::lookAt(glm::vec3(0.0f), spotLight.direction, glm::vec3(0.0f, 1.0f, 0.0f));
		spotLightMesh.transform.SetRotation(spotLightBaseRotation * glm::inverse(dir2rot));
		glm::vec2 cutoffAngle = glm::degrees(glm::acos(spotLight.cutoffAngle));
		ImGui::DragFloat("Inner cutoff angle", &cutoffAngle.x, 0.1f, cutoffAngle.y - 20.0f, cutoffAngle.y);
		ImGui::DragFloat("Outer cutoff angle", &cutoffAngle.y, 0.1f, cutoffAngle.x, 180.0f);
		spotLight.cutoffAngle = glm::cos(glm::radians(cutoffAngle));
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * (1.0f / 4.0f));
		ImGui::DragFloat("##3x", &spotLight.attenuation.x, 0.1f, 0.0f, 10.f); ImGui::SameLine();
		ImGui::DragFloat("##3y", &spotLight.attenuation.y, 0.01f, 0.0f, 1.f); ImGui::SameLine();
		ImGui::DragFloat("Attenuation##3z", &spotLight.attenuation.z, 0.001f, 0.0f, 0.1f);
		ImGui::PopItemWidth();
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
	// Cameras
	float aspectRatio;
	float cameraFOV = 56.0f;
	float cameraZoom = 1.0f;
	Hedge::Camera* camera;
	Hedge::PerspectiveCamera perspectiveCamera;
	Hedge::OrthographicCamera orthographicCamera;


	// Render settings
	bool wireframeMode;
	bool depthTest;
	bool faceCulling;
	bool blending;

	bool previousWireframeMode;
	bool previousDepthTest;
	bool previousFaceCulling;
	bool previousBlending;

	bool showAxes = true;
	bool showGrid = true;


	// Meshes
	Hedge::Mesh axesMesh;
	Hedge::Mesh gridMesh;

	Hedge::Mesh mesh;
	Hedge::Transform transform2;
	Hedge::Transform transform3;

	Hedge::Mesh squareMesh;

	Hedge::Mesh modelMesh;
	glm::vec3 translation = glm::vec3(0.0f);
	glm::vec3 rotate = glm::vec3(0.0f, 180.0f, 180.0f);
	float scale = 1.0f;


	// Lights
	Hedge::DirectionalLight directionalLight;

	Hedge::PointLight pointLight[2];
	Hedge::Mesh pointLightMeshes[2];

	Hedge::SpotLight spotLight;
	Hedge::Mesh spotLightMesh;
	glm::mat4 spotLightBaseRotation;


	// Mouse and Keyboard controls
	int lastX = 0;
	int lastY = 0;

	float xOffset = 0;
	float yOffset = 0;
	float zOffset = 0;

	float xRotation = 0;
	float yRotation = 0;
	float zRotation = 0;

	float movementSpeed = 2.5f / 1000.0f; // units/ms
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
