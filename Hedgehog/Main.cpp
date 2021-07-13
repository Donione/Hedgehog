#include <iostream>

// TODO: Applications using the Hedgehog engine should just include some Hedgehog.h header
//       and then create a concrete application class inheriting from the Hedgehog Application class
//       altough so far everything needed is in the Application anyway
// TODECIDE: Should the main function/entry point be a part of the engine or should it be up to the application (as it is here)?
#include <Application/Application.h>

#include <Renderer/DirectX12VertexArray.h>

#include <Component/Transform.h>
#include <Component/Light.h>
#include <Component/Mesh.h>
#include <Component/Scene.h>

#include <Model/Model.h>

#include <Animation/Animator.h>

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
//#include <spdlog/spdlog.h>
//#include "spdlog/sinks/stdout_color_sinks.h"

#include <imgui_internal.h> // for ImGui::PushItemFlag
#include <glm/gtc/type_ptr.hpp>

#include <Renderer/VulkanVertexArray.h>
#include <Renderer/VulkanBuffer.h>


struct Vertex
{
	float x, y, z, w, r, g, b, a, u, v, id;
};

void CreateFrustumVertices(Hedge::Frustum frustum, Vertex (&vertices)[8])
{
	// near clip face
	vertices[0] = { frustum.nearLeft, frustum.nearTop, -frustum.nearClip, 1.0f,			0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		-1.0f };	// top left
	vertices[1] = { frustum.nearRight, frustum.nearTop, -frustum.nearClip, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		-1.0f };	// top right
	vertices[2] = { frustum.nearLeft, frustum.nearBottom, -frustum.nearClip, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		-1.0f };	// bottom left
	vertices[3] = { frustum.nearRight, frustum.nearBottom, -frustum.nearClip, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		-1.0f };	// bottom right
	// far clip face
	vertices[4] = { frustum.farLeft, frustum.farTop, -frustum.farClip, 1.0f,			0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		-1.0f };	// top left
	vertices[5] = { frustum.farRight, frustum.farTop, -frustum.farClip, 1.0f,			0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		-1.0f };	// top right
	vertices[6] = { frustum.farLeft, frustum.farBottom, -frustum.farClip, 1.0f,			0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		-1.0f };	// bottom left
	vertices[7] = { frustum.farRight, frustum.farBottom, -frustum.farClip, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,		-1.0f };	// bottom right
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

		viewportDesc = { 0, 0, (int)Hedge::Application::GetInstance().GetWindow().GetWidth(), (int)Hedge::Application::GetInstance().GetWindow().GetHeight() };





		Hedge::ConstantBufferDescription frustumConstBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_segmentTransforms", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object, 65 },
		};

		auto frustumPrimitiveTopology = Hedge::PrimitiveTopology::Line;

		Hedge::BufferLayout frustumVertexBufferLayout =
		{
			{ Hedge::ShaderDataType::Float4, "a_position" },
			{ Hedge::ShaderDataType::Float4, "a_color" },
			{ Hedge::ShaderDataType::Float2, "a_textureCoordinates" },
			{ Hedge::ShaderDataType::Float,  "a_segmentID" }
		};

		std::string frustumVertexSrc;
		std::string frustumFragmentSrc;
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			frustumVertexSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
			frustumFragmentSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
		}
		else
		{
			frustumVertexSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLExampleVertexShader.glsl";
			frustumFragmentSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLExamplePixelShader.glsl";
		}

		aspectRatio = (float)Hedge::Application::GetInstance().GetWindow().GetWidth() / (float)Hedge::Application::GetInstance().GetWindow().GetHeight();

		auto camera = scene.CreateEntity("Scene Camera");
		auto& camera1camera = camera.Add<Hedge::Camera>(Hedge::Camera::CreatePerspective(aspectRatio, cameraFOV, 0.01f, 100.0f)); // camera space, +z goes into the screen
		//camera.Add<Hedge::Camera>(Hedge::Camera::CreateOrthographic(aspectRatio, 1.0f, 0.01f, 25.0f));
		auto& cameraTransform = camera.Add<Hedge::Transform>();
		cameraTransform.SetTranslation(glm::vec3(1.0f, 1.0f, 3.0f)); // world space, +z goes out of the screen
		cameraTransform.SetRotation(glm::vec3(-10.0f, 20.0f, 0.0f));

		// Now it seems obvious, but after emplacing more components into the registry, the references we got in the previous step might become
		// invalid because underlying containers might be reallocated
		CreateFrustumVertices(camera1camera.GetFrustum(), frustumVertices);
		auto& cameraMesh = camera.Add<Hedge::Mesh>(&frustumVertices[0].x, (unsigned int)sizeof(frustumVertices),
												   frustumIndices, (unsigned int)(sizeof(frustumIndices) / sizeof(unsigned int)),
												   frustumPrimitiveTopology, frustumVertexBufferLayout,
												   frustumConstBufferDesc,
												   frustumVertexSrc, frustumFragmentSrc);
		cameraMesh.enabled = false;

		auto camera2 = scene.CreateEntity("Camera 2");
		auto& camera2camera = camera2.Add<Hedge::Camera>(Hedge::Camera::CreatePerspective(aspectRatio, cameraFOV, 0.01f, 100.0f));
		//auto& camera2camera = camera2.Add<Hedge::Camera>(Hedge::Camera::CreateOrthographic(aspectRatio, 1.0f, 0.01f, 25.0f));
		camera2camera.SetPrimary(false);
		auto& camera2Transform = camera2.Add<Hedge::Transform>();
		camera2Transform.SetTranslation(glm::vec3(-1.0f, 1.0f, 3.0f));
		camera2Transform.SetRotation(glm::vec3(-10.0f, -20.0f, 0.0f));

		CreateFrustumVertices(camera2camera.GetFrustum(), frustumVertices);
		auto& camera2Mesh = camera2.Add<Hedge::Mesh>(&frustumVertices[0].x, (unsigned int)sizeof(frustumVertices),
													 frustumIndices, (unsigned int)(sizeof(frustumIndices) / sizeof(unsigned int)),
													 frustumPrimitiveTopology, frustumVertexBufferLayout,
													 frustumConstBufferDesc,
													 frustumVertexSrc, frustumFragmentSrc);
		camera2Mesh.enabled = false;





		std::string modelFilename = "c:\\Users\\Don\\Programming\\Hedgehog\\Hedgehog\\Asset\\Model\\bunny.tri";
		Hedge::Model bunnyModel;
		bunnyModel.LoadTri(modelFilename);

		auto modelPrimitiveTopology = Hedge::PrimitiveTopology::Triangle;
		Hedge::BufferLayout modelVertexBufferLayout =
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
			{ "u_numberOfPointLights", sizeof(int), Hedge::ConstantBufferUsage::Light },
			{ "u_pointLight", sizeof(Hedge::PointLight), Hedge::ConstantBufferUsage::Light, 3 },
			{ "u_spotLight", sizeof(Hedge::SpotLight), Hedge::ConstantBufferUsage::Light },
			{ "u_magnitude", sizeof(float), Hedge::ConstantBufferUsage::Object },
		};

		std::string modelVertexSrc;
		std::string modelFragmentSrc;
		std::string modelGeometrySrc;
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			modelVertexSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLModelVertexShader.glsl";
			modelFragmentSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLModelPixelShader.glsl";
			modelGeometrySrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLModelGeometryShader.glsl";
		}
		else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			modelVertexSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ModelShader.hlsl";
			modelFragmentSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ModelShader.hlsl";
			modelGeometrySrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ModelShader.hlsl";
		}

		bunnyEntity = scene.CreateEntity("Bunny");
		bunnyEntity.Add<Hedge::Mesh>(modelFilename,
									 modelPrimitiveTopology, modelVertexBufferLayout,
									 modelconstBufferDesc,
									 modelVertexSrc, modelFragmentSrc, modelGeometrySrc).enabled = true;
		auto& bunnyEntityTransform = bunnyEntity.Add<Hedge::Transform>();
		bunnyEntityTransform.SetTranslation(translation);
		bunnyEntityTransform.SetRotation(rotate);
		bunnyEntityTransform.SetUniformScale(scale);

		Hedge::BufferLayout modelInstanceVertexBufferLayout =
		{
			{ Hedge::ShaderDataType::Float3, "a_offset", 1 },
		};

		const int numX = 1;
		const int numY = 1;
		const int numZ = 1;
		float offsets[numX * numY * numZ * 3]{};

		int i = 0;
		for (int x = 0; x < numX; x++)
		{
			for (int y = 0; y < numY; y++)
			{
				for (int z = 0; z < numZ; z++)
				{
					offsets[i++] = (float)x * 2;
					offsets[i++] = (float)y * 2;
					offsets[i++] = (float)z * 2;
				}
			}
		}

		auto vertexBuffer = std::shared_ptr<Hedge::VertexBuffer>(Hedge::VertexBuffer::Create(modelInstanceVertexBufferLayout,
																							 offsets,
																							 sizeof(offsets)));
		bunnyEntity.Get<Hedge::Mesh>().Get()->AddVertexBuffer(vertexBuffer);
		bunnyEntity.Get<Hedge::Mesh>().Get()->SetInstanceCount(numX * numY * numZ);




		Hedge::ConstantBufferDescription constBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_segmentTransforms", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object, 65},
		};
		
		auto PrimitiveTopology = Hedge::PrimitiveTopology::Triangle;

		Hedge::BufferLayout vertexBufferLayout =
		{
			{ Hedge::ShaderDataType::Float4, "a_position" },
			{ Hedge::ShaderDataType::Float4, "a_color" },
			{ Hedge::ShaderDataType::Float2, "a_textureCoordinates"},
			{ Hedge::ShaderDataType::Float,  "a_segmentID" }
		};

		float vertices[] =
		{
			// 1x1x1 cube centered around the origin (0, 0, 0)
			// front face - white with some red on the bottom
			-0.5f,  0.5f,  0.5f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f,   0.0f, // top left
			 0.5f,  0.5f,  0.5f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f,   0.0f, // top right
			-0.5f, -0.5f,  0.5f, 1.0f,		1.0f, 0.8f, 0.8f, 1.0f,		0.0f, 0.0f,   0.0f, // bottom left
			 0.5f, -0.5f,  0.5f, 1.0f,		1.0f, 0.8f, 0.8f, 1.0f,		1.0f, 0.0f,   0.0f, // bottom right
			 // back face - black with some red on the bottom
			-0.5f,  0.5f, -0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,   0.0f, // top left
			 0.5f,  0.5f, -0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f,   0.0f, // top right
			-0.5f, -0.5f, -0.5f, 1.0f,		0.2f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,   0.0f, // bottom left
			 0.5f, -0.5f, -0.5f, 1.0f,		0.2f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f,   0.0f, // bottom right


			-0.5f,  0.5f + 1.5f,  0.5f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f,   1.0f, // top left
			 0.5f,  0.5f + 1.5f,  0.5f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f,   1.0f, // top right
			-0.5f, -0.5f + 1.5f,  0.5f, 1.0f,		1.0f, 0.8f, 0.8f, 1.0f,		0.0f, 0.0f,   1.0f, // bottom left
			 0.5f, -0.5f + 1.5f,  0.5f, 1.0f,		1.0f, 0.8f, 0.8f, 1.0f,		1.0f, 0.0f,   1.0f, // bottom right
			 // back face - black with some red on the bottom
			-0.5f,  0.5f + 1.5f, -0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,   1.0f, // top left
			 0.5f,  0.5f + 1.5f, -0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f,   1.0f, // top right
			-0.5f, -0.5f + 1.5f, -0.5f, 1.0f,		0.2f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,   1.0f, // bottom left
			 0.5f, -0.5f + 1.5f, -0.5f, 1.0f,		0.2f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f,   1.0f, // bottom right


			-0.5f,  0.5f + 3.0f,  0.5f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		0.0f, 1.0f,   2.0f, // top left
			 0.5f,  0.5f + 3.0f,  0.5f, 1.0f,		1.0f, 1.0f, 1.0f, 1.0f,		1.0f, 1.0f,   2.0f, // top right
			-0.5f, -0.5f + 3.0f,  0.5f, 1.0f,		1.0f, 0.8f, 0.8f, 1.0f,		0.0f, 0.0f,   2.0f, // bottom left
			 0.5f, -0.5f + 3.0f,  0.5f, 1.0f,		1.0f, 0.8f, 0.8f, 1.0f,		1.0f, 0.0f,   2.0f, // bottom right
			 // back face - black with some red on the bottom
			-0.5f,  0.5f + 3.0f, -0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,   2.0f, // top left
			 0.5f,  0.5f + 3.0f, -0.5f, 1.0f,		0.0f, 0.0f, 0.0f, 1.0f,		1.0f, 1.0f,   2.0f, // top right
			-0.5f, -0.5f + 3.0f, -0.5f, 1.0f,		0.2f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,   2.0f, // bottom left
			 0.5f, -0.5f + 3.0f, -0.5f, 1.0f,		0.2f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f,   2.0f, // bottom right
		};

		// Shaders
		std::string vertexSrc;
		std::string fragmentSrc;
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			vertexSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLExampleVertexShader.glsl";
			fragmentSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLExamplePixelShader.glsl";
		}
		else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			vertexSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
			fragmentSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
		}

		std::string vertexSrcTexture;
		std::string fragmentSrcTexture;
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			vertexSrcTexture = "..\\Hedgehog\\Asset\\Shader\\OpenGLNormalMapVertexShader.glsl";
			fragmentSrcTexture = "..\\Hedgehog\\Asset\\Shader\\OpenGLNormalMapPixelShader.glsl";
		}
		else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			vertexSrcTexture = "..\\Hedgehog\\Asset\\Shader\\DirectX12NormalMapShader.hlsl";
			fragmentSrcTexture = "..\\Hedgehog\\Asset\\Shader\\DirectX12NormalMapShader.hlsl";
		}

		unsigned int indices[36 * 3] = { 0,2,1, 1,2,3, 4,5,7, 4,7,6, 2,6,3, 3,6,7, 0,5,4, 0,1,5, 1,3,7, 1,7,5, 0,4,2, 2,4,6 };
		for (int i = 0; i < 36; i++)
		{
			indices[1 * 36 + i] = indices[i] + 8;
			indices[2 * 36 + i] = indices[i] + 16;
		}

		// We want to share this mesh for multiple render objects
		// Mesh component is just a bunch of smart pointers so we can just copy them for each entity
		// (not that there is a lot of data held within mesh components)
		//cubeMesh = Hedge::Mesh(vertices, sizeof(vertices),
		//					   indices, sizeof(indices) / sizeof(unsigned int),
		//					   PrimitiveTopology, vertexBufferLayout,
		//					   constBufferDesc,
		//					   vertexSrc, fragmentSrc);

		//auto cube1 = scene.CreateEntity("Cube 1");
		//cube1.Add<Hedge::Mesh>(cubeMesh).enabled = true;
		//auto& cube1Transform = cube1.Add<Hedge::Transform>();
		//cube1Transform.SetTranslation(glm::vec3(-2.0f, 0.0f, 0.0f));
		//cube1.Add<Hedge::Animator>(&animation);

		//auto cube2 = scene.CreateEntity("Cube 2");
		//cube2.Add<Hedge::Mesh>(mesh).enabled = false;
		//auto& cube2Transform = cube2.Add<Hedge::Transform>();
		//cube2Transform.Translate(glm::vec3(3.0f, 0.25f, 0.5f));
		//cube2Transform.Rotate(glm::vec3(0.0f, -20.0f, 180.0f));
		//cube2Transform.UniformScale(1.5f);

		//auto cube3 = scene.CreateEntity("Cube 3");
		//cube3.Add<Hedge::Mesh>(mesh).enabled = false;
		//auto& cube3Transform = cube3.Add<Hedge::Transform>();
		//cube3Transform.SetTranslation(glm::vec3(1.5f, 2.0f, -0.5f));
		//cube3Transform.SetRotation(glm::vec3(0.0f, -10.0f, 45.0f));
		//cube3Transform.SetScale(glm::vec3(0.5f, 1.0f, 0.5f));

		//auto childCube = cube3.CreateChild("Child Cube");
		//childCube.Add<Hedge::Mesh>(mesh).enabled = false;
		//auto& childCubeTransform = childCube.Add<Hedge::Transform>();
		//childCubeTransform.SetTranslation(glm::vec3(0.5f, 0.0f, 0.0f));
		//childCubeTransform.SetUniformScale(0.2f);
		//cube2.AddChild(cube3);





		Hedge::BufferLayout squareBufferLayout =
		{
			{ Hedge::ShaderDataType::Float3, "a_position" },
			{ Hedge::ShaderDataType::Float,  "a_textureSlot" },
			{ Hedge::ShaderDataType::Float2, "a_textureCoordinates"},
			{ Hedge::ShaderDataType::Float3, "a_normal" },
			{ Hedge::ShaderDataType::Float3, "a_tangent" },
			{ Hedge::ShaderDataType::Float3, "a_bitangent" },
			{ Hedge::ShaderDataType::Float4, "a_segmentIDs" },
			{ Hedge::ShaderDataType::Float4, "a_segmentWeigths" },
		};

		std::string textureFilename = "..\\Hedgehog\\Asset\\Texture\\diffuse.bmp";
		std::string normalMapFilename = "..\\Hedgehog\\Asset\\Texture\\normal.bmp";
		std::string specularMapFilename = "..\\Hedgehog\\Asset\\Texture\\specular.bmp";

		std::vector<Hedge::TextureDescription> textureDescriptions =
		{
			{ Hedge::TextureType::Diffuse, "..\\..\\Sponza-master\\textures\\spnza_bricks_a_diff.tga" },
			{ Hedge::TextureType::Normal, "..\\..\\Sponza-master\\textures\\spnza_bricks_a_ddn.tga" },
			{ Hedge::TextureType::Diffuse, textureFilename },
			{ Hedge::TextureType::Normal, normalMapFilename },
		};

		Hedge::ConstantBufferDescription squareConstBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_viewPos", sizeof(glm::vec3), Hedge::ConstantBufferUsage::Scene },
			{ "u_normalMapping", sizeof(int), Hedge::ConstantBufferUsage::Scene },
			{ "u_directionalLight", sizeof(Hedge::DirectionalLight), Hedge::ConstantBufferUsage::Light },
			{ "u_numberOfPointLights", sizeof(int), Hedge::ConstantBufferUsage::Light },
			{ "u_pointLight", sizeof(Hedge::PointLight), Hedge::ConstantBufferUsage::Light, 3 },
			{ "u_spotLight", sizeof(Hedge::SpotLight), Hedge::ConstantBufferUsage::Light },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_segmentTransforms", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object, 65 },
		};

		float squareVertices[] =
		{
			-0.5f,  0.5f + 0.0f,  0.0f,   1.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   0.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // top left      |
			 0.5f,  0.5f + 0.0f,  0.0f,   1.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   0.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // top right     } first triangle    |
			-0.5f, -0.5f + 0.0f,  0.0f,   1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   0.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // bottom left   |                   }  second trinagle
			 0.5f, -0.5f + 0.0f,  0.0f,   1.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   0.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // bottom right                      |

			-0.5f,  0.5f + 1.5f,  0.0f,   1.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   1.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // top left      |
			 0.5f,  0.5f + 1.5f,  0.0f,   1.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   1.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // top right     } first triangle    |
			-0.5f, -0.5f + 1.5f,  0.0f,   1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   1.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // bottom left   |                   }  second trinagle
			 0.5f, -0.5f + 1.5f,  0.0f,   1.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   1.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // bottom right                      |

			-0.5f,  0.5f + 3.0f,  0.0f,   1.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   2.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // top left      |
			 0.5f,  0.5f + 3.0f,  0.0f,   1.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   2.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // top right     } first triangle    |
			-0.5f, -0.5f + 3.0f,  0.0f,   1.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   2.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // bottom left   |                   }  second trinagle
			 0.5f, -0.5f + 3.0f,  0.0f,   1.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,   2.0f, -1.0f, -1.0f, -1.0f,   1.0f, 1.0f, 1.0f, 1.0f, // bottom right                      |
		};
		unsigned int indicesSquare[] = { 0,2,1, 1,2,3,   4,6,5, 5,6,7,   8,10,9, 9,10,11 };

		int stride = squareBufferLayout.GetStride() / sizeof(float);
		for (int i = 0; i < 6 * 3; i+=3)
		{
			glm::vec3 pos1, pos2, pos3;
			glm::vec2 uv1, uv2, uv3;

			pos1 = { squareVertices[indicesSquare[i + 0] * stride + 0], squareVertices[indicesSquare[i + 0] * stride + 1], squareVertices[indicesSquare[i + 0] * stride + 2] };
			pos2 = { squareVertices[indicesSquare[i + 1] * stride + 0], squareVertices[indicesSquare[i + 1] * stride + 1], squareVertices[indicesSquare[i + 1] * stride + 2] };
			pos3 = { squareVertices[indicesSquare[i + 2] * stride + 0], squareVertices[indicesSquare[i + 2] * stride + 1], squareVertices[indicesSquare[i + 2] * stride + 2] };

			uv1 = { squareVertices[indicesSquare[i + 0] * stride + 4], squareVertices[indicesSquare[i + 0] * stride + 5] };
			uv2 = { squareVertices[indicesSquare[i + 1] * stride + 4], squareVertices[indicesSquare[i + 1] * stride + 5] };
			uv3 = { squareVertices[indicesSquare[i + 2] * stride + 4], squareVertices[indicesSquare[i + 2] * stride + 5] };

			glm::vec3 edge1 = pos2 - pos1;
			glm::vec3 edge2 = pos3 - pos1;
			glm::vec2 deltaUV1 = uv2 - uv1;
			glm::vec2 deltaUV2 = uv3 - uv1;

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

			// tangent
			squareVertices[indicesSquare[i + 0] * stride + 9] = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			squareVertices[indicesSquare[i + 0] * stride + 10] = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			squareVertices[indicesSquare[i + 0] * stride + 11] = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			// bitangent
			squareVertices[indicesSquare[i + 0] * stride + 12] = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			squareVertices[indicesSquare[i + 0] * stride + 13] = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			squareVertices[indicesSquare[i + 0] * stride + 14] = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

			// tangent
			squareVertices[indicesSquare[i + 1] * stride + 9] = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			squareVertices[indicesSquare[i + 1] * stride + 10] = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			squareVertices[indicesSquare[i + 1] * stride + 11] = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			// bitangent
			squareVertices[indicesSquare[i + 1] * stride + 12] = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			squareVertices[indicesSquare[i + 1] * stride + 13] = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			squareVertices[indicesSquare[i + 1] * stride + 14] = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

			// tangent
			squareVertices[indicesSquare[i + 2] * stride + 9] = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			squareVertices[indicesSquare[i + 2] * stride + 10] = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			squareVertices[indicesSquare[i + 2] * stride + 11] = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			// bitangent
			squareVertices[indicesSquare[i + 2] * stride + 12] = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			squareVertices[indicesSquare[i + 2] * stride + 13] = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			squareVertices[indicesSquare[i + 2] * stride + 14] = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		}

		//std::string sponzaFilename = "..\\..\\Sponza-master\\sponza.obj";

		//Hedge::Model sponzaModel;
		//sponzaModel.LoadObj(sponzaFilename);

		//std::vector<Hedge::TextureDescription> squareTextureDescription;
		//for (int i = 0; i < 50; i++)
		//{
		//	squareTextureDescription.push_back(textureDescriptions[i % 4]);
		//}

		//squareMesh = Hedge::Mesh(squareVertices, sizeof(squareVertices),
		//						 indicesSquare, sizeof(indicesSquare) / sizeof(unsigned int),
		//						 PrimitiveTopology, squareBufferLayout,
		//						 squareConstBufferDesc,
		//						 vertexSrcTexture, fragmentSrcTexture, "",
		//						 squareTextureDescription);
		////squareTransform.SetTranslation(glm::vec3(-1.0f, 2.0f, 0.0f));
		//squareTransform.SetTranslation(glm::vec3(2.0f, 0.0f, 0.0f));

		//auto square = scene.CreateEntity("Square");
		//square.Add<Hedge::Mesh>(squareMesh).enabled = true;
		//square.Add<Hedge::Transform>(squareTransform);
		//square.Add<Hedge::Animator>(&animation);



		//spozaTestEntity = scene.CreateEntity("Sponza");
		//auto& spoznaMesh = spozaTestEntity.Add<Hedge::Mesh>(sponzaModel.GetVertices(), sponzaModel.GetSizeOfVertices(),
		//													sponzaModel.GetIndices(), sponzaModel.GetNumberOfIndices(),
		//													Hedge::PrimitiveTopology::Triangle, squareBufferLayout,
		//													squareConstBufferDesc,
		//													vertexSrcTexture, fragmentSrcTexture, "",
		//													sponzaModel.GetTextureDescription(),
		//													sponzaModel.GetGroups());
		//spoznaMesh.enabled = false;
		//auto& spozaTransform = spozaTestEntity.Add<Hedge::Transform>();
		//spozaTransform.SetUniformScale(0.01f);

		//Hedge::BufferLayout TBNBL =
		//{
		//	{ Hedge::ShaderDataType::Float4, "a_position" },
		//	{ Hedge::ShaderDataType::Float4, "a_color" },
		//	{ Hedge::ShaderDataType::Float2, "a_textureCoordinates" },
		//	{ Hedge::ShaderDataType::Float,  "a_segmentID" },
		//};

		//if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		//{
		//	vertexSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLExampleVertexShader.glsl";
		//	fragmentSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLExamplePixelShader.glsl";
		//}
		//else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		//{
		//	vertexSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
		//	fragmentSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
		//}

		//constBufferDesc =
		//{
		//	{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
		//	{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
		//	{ "u_segmentTransforms", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object, 65 },
		//};

		//sponzaDebugEntity = scene.CreateEntity("Sponza Debug");
		//auto& sponzaDebugMesh = sponzaDebugEntity.Add<Hedge::Mesh>(sponzaModel.GetTBNVertices(), sponzaModel.GetSizeOfTBNVertices(),
		//														   sponzaModel.GetTBNIndices(), sponzaModel.GetNumberOfTBNIndices(),
		//														   Hedge::PrimitiveTopology::Line, TBNBL,
		//														   constBufferDesc,
		//														   vertexSrc, fragmentSrc);
		//sponzaDebugMesh.enabled = false;
		//sponzaDebugEntity.Add<Hedge::Transform>().SetUniformScale(0.01f);

		//std::vector<Hedge::TextureDescription> vampireTextureDescriptions;
		//for (int i = 0; i < 25; i++)
		//{
		//	vampireTextureDescriptions.push_back({ Hedge::TextureType::Diffuse, "..\\..\\vampire\\textures\\Vampire_diffuse.png" });
		//	vampireTextureDescriptions.push_back({ Hedge::TextureType::Normal, "..\\..\\vampire\\textures\\Vampire_diffuse.png" });
		//}

		//vampireModel.LoadDae("..\\..\\vampire\\dancing_vampire.dae");
		//vampireEntity = scene.CreateEntity("Vampire");
		//auto& vampireMesh = vampireEntity.Add<Hedge::Mesh>(vampireModel.GetVertices(), vampireModel.GetSizeOfVertices(),
		//												   vampireModel.GetIndices(), vampireModel.GetNumberOfIndices(),
		//												   Hedge::PrimitiveTopology::Triangle, squareBufferLayout,
		//												   squareConstBufferDesc,
		//												   vertexSrcTexture, fragmentSrcTexture, "",
		//												   vampireTextureDescriptions
		//												   );
		//vampireMesh.enabled = true;
		//auto& vampireTransform = vampireEntity.Add<Hedge::Transform>();
		//vampireTransform.SetUniformScale(0.01f);
		//vampireEntity.Add<Hedge::Animator>(vampireModel.GetAnimation());





		// Lights
		// Directional Lights
		auto dirLightEntity = scene.CreateEntity("Directional Light");
		auto& dirLightLight = dirLightEntity.Add<Hedge::DirectionalLight>();
		dirLightLight.color = glm::vec3(0.0f); // glm::vec3(1.0f, 0.8f, 0.0f);
		dirLightLight.direction = glm::vec3(0.0f, 0.0f, -1.0f);

		// Pointlights
		modelFilename = "..\\Hedgehog\\Asset\\Model\\koule.tri";

		auto lightPrimitiveTopology = Hedge::PrimitiveTopology::Triangle;

		Hedge::BufferLayout lightVertexBufferArrayLayout =
		{
			{ Hedge::ShaderDataType::Float3, "a_position" },
			{ Hedge::ShaderDataType::Float3, "a_normal" },
		};

		Hedge::ConstantBufferDescription lightconstBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_lightColor", sizeof(glm::vec3), Hedge::ConstantBufferUsage::Object },
		};

		std::string lightVertexSrc;
		std::string lightFragmentSrc;
		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			lightVertexSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ModelExampleShader.hlsl";
			lightFragmentSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ModelExampleShader.hlsl";
		}
		else
		{
			lightVertexSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLModelExampleVertexShader.glsl";
			lightFragmentSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLModelExamplePixelShader.glsl";
		}
		
		auto pointLightMesh = Hedge::Mesh(modelFilename,
										  lightPrimitiveTopology, lightVertexBufferArrayLayout,
										  lightconstBufferDesc,
										  lightVertexSrc, lightFragmentSrc);
		
		pointLight1 = scene.CreateEntity("Point Light 1");
		auto& pointLight1Light = pointLight1.Add<Hedge::PointLight>();
		pointLight1Light.color = (glm::vec3(1.0f, 1.0f, 1.0f));
		pointLight1Light.attenuation = (glm::vec3(1.0f, 0.027f, 0.0028f));
		pointLight1Light.position = (glm::vec3(0.0f, 0.0f, 1.0f));
		pointLight1.Add<Hedge::Mesh>(pointLightMesh).enabled = false;
		auto& pointLight1Transform = pointLight1.Add<Hedge::Transform>();
		pointLight1Transform.SetUniformScale(0.1f);
		pointLight1Transform.SetTranslation(pointLight1Light.position);
		
		auto pointLight2 = scene.CreateEntity("Point Light 2");
		auto& pointLight2Light = pointLight2.Add<Hedge::PointLight>();
		pointLight2Light.color = glm::vec3(0.0f, 0.0f, 0.0f);
		pointLight2Light.attenuation = glm::vec3(1.0f, 0.027f, 0.0028f);
		pointLight2Light.position = glm::vec3(0.0f, 2.0f, 0.0f);
		pointLight2.Add<Hedge::Mesh>(pointLightMesh).enabled = false;
		auto& pointLight2Transform = pointLight2.Add<Hedge::Transform>();
		pointLight2Transform.SetUniformScale(0.1f);
		pointLight2Transform.SetTranslation(pointLight2Light.position);
		
		auto pointLight3 = scene.CreateEntity("Point Light 3");
		auto& newLight = pointLight3.Add<Hedge::PointLight>();
		newLight.color = glm::vec3(0.0f, 0.0f, 0.0f);
		newLight.position = glm::vec3(0.0f, -2.0f, 0.0f);
		pointLight3.Add<Hedge::Mesh>(pointLightMesh).enabled = false;
		auto& newTransform = pointLight3.Add<Hedge::Transform>();
		newTransform.SetTranslation(newLight.position);
		newTransform.SetUniformScale(0.1f);

		// Spotlight
		modelFilename = "..\\Hedgehog\\Asset\\Model\\valec.tri";

		auto spotlight = scene.CreateEntity("Spotlight");
		auto& spotLightLight = spotlight.Add<Hedge::SpotLight>();
		spotLightLight.color = glm::vec3(0.0f, 0.0f, 0.0f);
		spotLightLight.attenuation = glm::vec3(1.0f, 0.027f, 0.0028f);
		spotLightLight.position = glm::vec3(0.0f, 0.0f, 2.0f);
		spotLightLight.direction = glm::vec3(0.0f, 0.0f, -1.0f);
		spotlight.Add<Hedge::Mesh>(modelFilename,
								   lightPrimitiveTopology, lightVertexBufferArrayLayout,
								   lightconstBufferDesc,
								   lightVertexSrc, lightFragmentSrc).enabled = false;
		auto& spotLightTransform = spotlight.Add<Hedge::Transform>();
		spotLightTransform.SetUniformScale(0.1f);
		spotLightTransform.SetTranslation(spotLightLight.position);

		spotLightBaseRotation = glm::mat4(1.0f);
		spotLightBaseRotation = glm::rotate(spotLightBaseRotation, glm::radians(9.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		spotLightBaseRotation = glm::rotate(spotLightBaseRotation, glm::radians(-13.75f), glm::vec3(0.0f, 1.0f, 0.0f));

		spotLightTransform.SetRotation(spotLightBaseRotation);





		// Axes lines
		auto axesPrimitiveTopology = Hedge::PrimitiveTopology::Line;

		Hedge::BufferLayout axesBL =
		{
			{ Hedge::ShaderDataType::Float4, "a_position" },
			{ Hedge::ShaderDataType::Float4, "a_color" },
			{ Hedge::ShaderDataType::Float2, "a_textureCoordinates" },
			{ Hedge::ShaderDataType::Float,  "a_segmentID" },
		};

		Vertex axesVertices[6] =
		{
			{ -100.0f, 0.0f, 0.0f, 1.0f,	1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,   -1.0f }, // X axis
			{ 100.0f, 0.0f, 0.0f, 1.0f,		1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 0.0f,   -1.0f },
			{ 0.0f, -100.0f, 0.0f, 1.0f,	0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f,   -1.0f }, // Y axis
			{ 0.0f, 100.0f, 0.0f, 1.0f,		0.0f, 1.0f, 0.0f, 1.0f,		0.0f, 0.0f,   -1.0f },
			{ 0.0f, 0.0f, -100.0f, 1.0f,	0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f,   -1.0f }, // Z axis
			{ 0.0f, 0.0f, 100.0f, 1.0f,		0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f,   -1.0f },
		};

		unsigned int axesIndices[] = { 0, 1, 2, 3, 4, 5 };

		int index = 0;

		for (int x = -100; x <= 100; x++)
		{
			gridVertices[index++] = { (float)x, 0.0f, -100.0f, 1.0f,	0.3f, 0.3f, 0.3f, 1.0f,		0.0f, 0.0f,   -1.0f };
			gridVertices[index++] = { (float)x, 0.0f,  100.0f, 1.0f,	0.3f, 0.3f, 0.3f, 1.0f,		0.0f, 0.0f,   -1.0f };
		}

		for (int z = -100; z <= 100; z++)
		{
			gridVertices[index++] = { -100.0f, 0.0f, (float)z, 1.0f,	0.3f, 0.3f, 0.3f, 1.0f,		0.0f, 0.0f,   -1.0f };
			gridVertices[index++] = {  100.0f, 0.0f, (float)z, 1.0f,	0.3f, 0.3f, 0.3f, 1.0f,		0.0f, 0.0f,   -1.0f };
		}

		assert(index == numVertices);

		unsigned int gridIndices[numVertices];
		for (int i = 0; i < numVertices; i++)
		{
			gridIndices[i] = i;
		}

		if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::OpenGL)
		{
			vertexSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLExampleVertexShader.glsl";
			fragmentSrc = "..\\Hedgehog\\Asset\\Shader\\OpenGLExamplePixelShader.glsl";
		}
		else if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
		{
			vertexSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
			fragmentSrc = "..\\Hedgehog\\Asset\\Shader\\DirectX12ExampleShader.hlsl";
		}

		constBufferDesc =
		{
			{ "u_ViewProjection", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_Transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_segmentTransforms", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object, 65},
		};

		axesEntity = scene.CreateEntity("Axes");
		axesEntity.Add<Hedge::Mesh>(&axesVertices[0].x, (unsigned int)sizeof(axesVertices),
									axesIndices, (unsigned int)(sizeof(axesIndices) / sizeof(unsigned int)),
									axesPrimitiveTopology, axesBL,
									constBufferDesc,
									vertexSrc, fragmentSrc);
		axesEntity.Add<Hedge::Transform>();
		
		gridEntity = scene.CreateEntity("Grid");
		auto& gridMesh = gridEntity.Add<Hedge::Mesh>(&gridVertices[0].x, (unsigned int)sizeof(gridVertices),
													 gridIndices, (unsigned int)(sizeof(gridIndices) / sizeof(unsigned int)),
													 axesPrimitiveTopology, axesBL,
													 constBufferDesc,
													 vertexSrc, fragmentSrc);
		gridEntity.Add<Hedge::Transform>();

		for (int i = 0; i < 65; i++)
		{
			ones.push_back(glm::mat4(1.0f));
		}
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

		auto primaryCamera = scene.GetPrimaryCamera();

		if (GetKeyState(VK_SPACE) < 0)
		{
			if (primaryCamera)
			{
				primaryCamera.Get<Hedge::Transform>().SetTranslation({ 0.0f, 0.0f, 0.0f });
				primaryCamera.Get<Hedge::Transform>().SetRotation({ 0.0f, 0.0f, 0.0f });
			}
		}

		if (primaryCamera)
		{
			primaryCamera.Get<Hedge::Transform>().Translate(glm::vec3(xOffset * movementSpeed * (float)duration.count(), yOffset * scrollSpeed, zOffset * movementSpeed * (float)duration.count()));
			primaryCamera.Get<Hedge::Transform>().Rotate(glm::vec3(mouseSpeed * xRotation, mouseSpeed * yRotation, zRotation * rotationSpeed * (float)duration.count()));
		
			pointLight1.Get<Hedge::PointLight>().position = primaryCamera.Get<Hedge::Transform>().GetTranslation();
			pointLight1.Get<Hedge::Transform>().SetTranslation(primaryCamera.Get<Hedge::Transform>().GetTranslation());
		}

		xOffset = 0;
		yOffset = 0;
		zOffset = 0;

		xRotation = 0;
		yRotation = 0;
		zRotation = 0;

		Hedge::Renderer::BeginScene(primaryCamera);
		{
			//squareMesh.GetShader()->UploadConstant("u_normalMapping", (int)normalMapping);
			//spozaTestEntity.Get<Hedge::Mesh>().GetShader()->UploadConstant("u_normalMapping", (int)normalMapping);
			//vampireEntity.Get<Hedge::Mesh>().GetShader()->UploadConstant("u_normalMapping", (int)normalMapping);

			//spozaTestEntity.Get<Hedge::Mesh>().GetShader()->UploadConstant("u_segmentTransforms", ones);

			bunnyEntity.Get<Hedge::Mesh>().GetShader()->UploadConstant("u_magnitude", magnitude);

			scene.OnUpdate(duration);
			
			// Order matters when we want to blend
			// TODO there needs to be a way to sort meshes for blending, especially when using the EnTT registry
			//Hedge::Renderer::Submit(squareMesh.Get(), squareTransform.Get());
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
			// But seriously, this needs to be done properly, when iterating meshes through the registry, shared meshes are updated multiple times
			// per frame which breaks the pipelines in flight unless the buffer of PSOs is large enough
			if (Hedge::Renderer::GetAPI() == Hedge::RendererAPI::API::DirectX12)
			{
				//std::dynamic_pointer_cast<Hedge::DirectX12VertexArray>(squareMesh.Get())->UpdateRenderSettings();

				scene.UpdateRenderSettings();
			}
		}
	}

	void OnGuiUpdate() override
	{
		// 2nd part of workaround for docked viewport layout
		// By pushing these style colors before calling the DockSpace and poping them after we begin the viewport window
		// we get the desired output of transparent viewport window and other windows with their original background colors
		// Again, not exactly sure why and how this works
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		static bool viewportWindowOpen = false;
		ImGui::Begin("Viewport", &viewportWindowOpen, ImGuiWindowFlags_NoMove);
		ImGui::PopStyleColor(2);


		auto vpPos = ImGui::GetWindowViewport()->Pos;
		auto vpSize = ImGui::GetWindowViewport()->Size;
		auto pos = ImGui::GetWindowPos();
		auto size = ImGui::GetWindowSize();
		ViewportDesc currentViewportDec =
		{
			(int)(pos.x - vpPos.x),
			(int)(pos.y - vpPos.y),
			(int)size.x,
			(int)size.y
		};
		// TODO in OpenGL we're getting a flicker on the first frame since the viewport isn't set to the viewport window size
		// might want to clean this up
		if (currentViewportDec != viewportDesc)
		{
			viewportDesc = currentViewportDec;

			Hedge::RenderCommand::SetViewport(viewportDesc.x, viewportDesc.y, viewportDesc.width, viewportDesc.height);
			Hedge::RenderCommand::SetScissor(viewportDesc.x, viewportDesc.y, viewportDesc.width, viewportDesc.height);
			aspectRatio = (float)size.x / (float)size.y;
			scene.GetPrimaryCamera().Get<Hedge::Camera>().SetAspectRatio(aspectRatio);
		}

		viewportHovered = ImGui::IsMouseHoveringRect(pos, ImVec2(pos.x + size.x, pos.y + size.y));
		
		ImGui::End();


		ImGui::Begin("Window");
		ImGui::Text("Client Area Size: %u %u", Hedge::Application::GetInstance().GetWindow().GetWidth(), Hedge::Application::GetInstance().GetWindow().GetHeight());
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
		ImGui::SameLine(); ImGui::Checkbox("Depth Test", &depthTest);
		ImGui::SameLine(); ImGui::Checkbox("Face Culling", &faceCulling);
		ImGui::SameLine(); ImGui::Checkbox("Blending", &blending);

		ImGui::Checkbox("Use Normal Mapping", &normalMapping);

		ImGui::End();


		ImGui::Begin("Scene");

		ImGui::SliderFloat("Magnitude", &magnitude, 0.0f, 1.0f);

		ImGui::Checkbox("Show Axes", &axesEntity.Get<Hedge::Mesh>().enabled);
		ImGui::SameLine(); ImGui::Checkbox("Show Grid", &gridEntity.Get<Hedge::Mesh>().enabled);

		ImGui::Separator();

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("Meshes"))
		{
			auto view = scene.registry.view <std::string, Hedge::Mesh, Hedge::Transform>(entt::exclude<Hedge::DirectionalLight, Hedge::SpotLight, Hedge::PointLight, Hedge::Camera>);
			for (auto [entity, name, mesh, transform] : view.each())
			{
				if (axesEntity.Is(entity)
					|| gridEntity.Is(entity))
				{
					continue;
				}

				char label[256] = {};

				sprintf_s(label, "%s", name.c_str());
				if (ImGui::TreeNode(label))
				{
					ImGui::Checkbox(label, &mesh.enabled);

					transform.CreateGuiControls();

					if (scene.registry.has<Hedge::Animator>(entity))
					{
						ImGui::Separator();
						ImGui::Text("Animation");

						scene.registry.get<Hedge::Animator>(entity).CreateGuiControls();
					}

					auto& groups = mesh.Get()->GetGroups();

					if (!groups.empty())
					{
						if (ImGui::TreeNode("Groups"))
						{
							if (ImGui::Button("Enable All"))
							{
								for (auto& [group, distance] : groups)
								{
									group.enabled = true;
								}
							}

							ImGui::SameLine();
							if (ImGui::Button("Disable All"))
							{
								for (auto& [group, distance] : groups)
								{
									group.enabled = false;
								}
							}

							for (auto& [group, distance] : groups)
							{
								ImGui::Checkbox(group.name.c_str(), &group.enabled);
							}

							ImGui::TreePop();
						} // groups node
					} // groups

					ImGui::TreePop();
				} // Mesh node
			} // meshes

			ImGui::TreePop();
		} // Meshes node

		ImGui::Separator();

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("Lights"))
		{
			if (ImGui::TreeNode("Directional Lights"))
			{
				auto view = scene.registry.view<std::string, Hedge::DirectionalLight>();

				for (auto [entity, name, light] : view.each())
				{
					ImGui::SetNextItemOpen(true, ImGuiCond_Once);
					if (ImGui::TreeNode(name.c_str()))
					{
						light.CreateGuiControls();

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			} // Directional Lights node

			if (ImGui::TreeNode("Point Lights"))
			{
				ImGui::SliderInt("# of PLights used", &scene.plUsed, 0, 3);

				auto view = scene.registry.view<std::string, Hedge::Mesh, Hedge::Transform, Hedge::PointLight>();
				for (auto [entity, name, mesh, transform, light] : view.each())
				{
					ImGui::SetNextItemOpen(true, ImGuiCond_Once);
					if (ImGui::TreeNode(name.c_str()))
					{
						ImGui::Checkbox("Render Mesh", &mesh.enabled);

						ImGui::SameLine();
						ImGui::PushItemWidth(150.0f);
						transform.CreateGuiControls(false, false, true);
						ImGui::PopItemWidth();

						if (light.CreateGuiControls())
						{
							transform.SetTranslation(light.position);
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			} // Point Lights node

			if (ImGui::TreeNode("Spot Lights"))
			{
				auto view = scene.registry.view<std::string, Hedge::Mesh, Hedge::Transform, Hedge::SpotLight>();

				for (auto [entity, name, mesh, transform, light] : view.each())
				{
					ImGui::SetNextItemOpen(true, ImGuiCond_Once);
					if (ImGui::TreeNode(name.c_str()))
					{
						ImGui::Checkbox("Render Mesh", &mesh.enabled);

						if (light.CreateGuiControls())
						{
							transform.SetTranslation(light.position);
							auto dir2rot = glm::lookAt(glm::vec3(0.0f), light.direction, glm::vec3(0.0f, 1.0f, 0.0f));
							transform.SetRotation(spotLightBaseRotation * glm::inverse(dir2rot));
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			} // Spot Lights node

			ImGui::TreePop();
		} // Lights node

		ImGui::Separator();

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode("Cameras"))
		{
			auto view = scene.registry.view<std::string, Hedge::Camera, Hedge::Transform, Hedge::Mesh>();
			for (auto [entity, name, camera, transform, mesh] : view.each())
			{
				if (ImGui::TreeNode(name.c_str()))
				{
					if (camera.CreateGuiControls())
					{
						auto& frustum = camera.GetFrustum();
						CreateFrustumVertices(frustum, frustumVertices);

						mesh.Get()->GetVertexBuffers().at(0)->SetData(&frustumVertices[0].x, (unsigned int)sizeof(frustumVertices));
					}
					transform.CreateGuiControls(true, true, false);
					
					ImGui::Checkbox("Render Frustum", &mesh.enabled);

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		} // Cameras node
		
		ImGui::End(); // Scene window
	}

	void OnMessage(const Hedge::Message& message) override
	{
		if (message.GetMessageType() == Hedge::MessageType::MouseScrolled
			&& viewportHovered)
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
	}

private:
	Hedge::Scene scene;

	Hedge::Entity axesEntity;
	Hedge::Entity gridEntity;

	bool viewportHovered = false;


	struct ViewportDesc
	{
		int x;
		int y;
		int width;
		int height;

		bool operator!=(const ViewportDesc& other)
		{
			return x != other.x || y != other.y || width != other.width || height != other.height;
		}
	};

	ViewportDesc viewportDesc;


	// Cameras
	float aspectRatio;
	float cameraFOV = 56.0f;
	float cameraZoom = 1.0f;


	// Render settings
	bool wireframeMode;
	bool depthTest;
	bool faceCulling;
	bool blending;

	bool previousWireframeMode;
	bool previousDepthTest;
	bool previousFaceCulling;
	bool previousBlending;

	Vertex frustumVertices[8];
	unsigned int frustumIndices[8*3] =
	{
		0, 1, 1, 3, 3, 2, 2, 0, // near clip face
		4, 5, 5, 7, 7, 6, 6, 4, // far clip face
		0, 4, 1, 5, 2, 6, 3, 7, // connecting lines
	};

	static const int numVertices = 804;
	Vertex gridVertices[numVertices];

	Hedge::Mesh cubeMesh;
	Hedge::Mesh squareMesh;
	Hedge::Transform squareTransform;

	Hedge::Entity spozaTestEntity;
	Hedge::Entity sponzaDebugEntity;

	glm::vec3 translation = glm::vec3(0.0f);
	glm::vec3 rotate = glm::vec3(0.0f, 180.0f, 180.0f);
	float scale = 1.0f;
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

	bool normalMapping = false;
	std::shared_ptr<Hedge::Texture> normalMap;
	std::shared_ptr<Hedge::Texture> specularMap;

	Hedge::Entity pointLight1;

	Hedge::Animation animation;

	std::vector<glm::mat4> ones;

	Hedge::Entity vampireEntity;
	Hedge::Model vampireModel;

	Hedge::Entity bunnyEntity;
	float magnitude = 0.0f;
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

			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
			Hedge::RenderCommand::SetClearColor({ clear_color.x, clear_color.y, clear_color.z, 1.0f });

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}
	}

	void OnMessage(const Hedge::Message& message) override
	{
	}

private:
	// Our state
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool show_demo_window = false;
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


class VulkanTestLayer : public Hedge::Layer
{
public:
	VulkanTestLayer(bool enable = true) :
		Layer("Vulkan Test Layer", enable)
	{
		previousWireframeMode = wireframeMode = Hedge::RenderCommand::GetWireframeMode();
		previousDepthTest = depthTest = Hedge::RenderCommand::GetDepthTest();
		previousFaceCulling = faceCulling = Hedge::RenderCommand::GetFaceCulling();
		previousBlending = blending = Hedge::RenderCommand::GetBlending();

		viewportDesc = { 0, 0, (int)Hedge::Application::GetInstance().GetWindow().GetWidth(), (int)Hedge::Application::GetInstance().GetWindow().GetHeight() };
	
		aspectRatio = (float)Hedge::Application::GetInstance().GetWindow().GetWidth() / (float)Hedge::Application::GetInstance().GetWindow().GetHeight();

		auto camera = scene.CreateEntity("Scene Camera");
		auto& camera1camera = camera.Add<Hedge::Camera>(Hedge::Camera::CreatePerspective(aspectRatio, cameraFOV, 0.01f, 100.0f));
		auto& cameraTransform = camera.Add<Hedge::Transform>();
		cameraTransform.SetTranslation(glm::vec3(0.0f, 0.0f, 3.0f));


		shader.reset(Hedge::Shader::Create("..\\Hedgehog\\Asset\\Shader\\VulkanExampleVertexShader.spv",
										   "..\\Hedgehog\\Asset\\Shader\\VulkanExamplePixelShader.spv"));

		Hedge::ConstantBufferDescription constantBufferDescription =
		{
			{ "u_projectionView", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Scene },
			{ "u_transform", sizeof(glm::mat4), Hedge::ConstantBufferUsage::Object },
			{ "u_dummy", sizeof(glm::vec2), Hedge::ConstantBufferUsage::Scene },
		};

		shader->SetupConstantBuffers(constantBufferDescription);

		vertexArray.reset(Hedge::VertexArray::Create(shader,
													 Hedge::PrimitiveTopology::Triangle,
													 {},
													 {}));

		Hedge::BufferLayout bufferLayout1 =
		{
			{ Hedge::ShaderDataType::Float2, "a_position" },
		};

		Hedge::BufferLayout bufferLayout2 =
		{
			{ Hedge::ShaderDataType::Float3, "a_color" },
		};

		Hedge::BufferLayout bufferLayout3 =
		{
			{ Hedge::ShaderDataType::Float, "a_offset", 1 },
		};

		float vertices1[] =
		{
			 0.0f, -0.5f,
			 0.5f,  0.5f,
			-0.5f,  0.5f,
		};

		float vertices2[] =
		{
			1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 1.0f,
		};

		float vertices3[] =
		{
			0.0f,
			-1.0f,
			0.25f,
		};

		vertexBuffer1.reset(Hedge::VertexBuffer::Create(bufferLayout1, vertices1, sizeof(vertices1)));
		vertexBuffer2.reset(Hedge::VertexBuffer::Create(bufferLayout2, vertices2, sizeof(vertices2)));
		vertexBuffer3.reset(Hedge::VertexBuffer::Create(bufferLayout3, vertices3, sizeof(vertices3)));

		vertexArray->AddVertexBuffer(vertexBuffer1);
		vertexArray->AddVertexBuffer(vertexBuffer2);
		vertexArray->AddVertexBuffer(vertexBuffer3);

		vertexArray->SetInstanceCount(3);

		unsigned int indices[] = { 0, 1, 2 };

		indexBuffer.reset(Hedge::IndexBuffer::Create(indices, 3));

		vertexArray->AddIndexBuffer(indexBuffer);
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

		if (GetKeyState(0x45) < 0) // 'E'
		{
			zRotation--;
		}

		if (GetKeyState(0x51) < 0) // 'Q'
		{
			zRotation++;
		}

		auto primaryCamera = scene.GetPrimaryCamera();

		if (GetKeyState(VK_SPACE) < 0)
		{
			if (primaryCamera)
			{
				primaryCamera.Get<Hedge::Transform>().SetTranslation({ 0.0f, 0.0f, 0.0f });
				primaryCamera.Get<Hedge::Transform>().SetRotation({ 0.0f, 0.0f, 0.0f });
			}
		}

		if (primaryCamera)
		{
			primaryCamera.Get<Hedge::Transform>().Translate(glm::vec3(xOffset * movementSpeed * (float)duration.count(), yOffset * scrollSpeed, zOffset * movementSpeed * (float)duration.count()));
			primaryCamera.Get<Hedge::Transform>().Rotate(glm::vec3(mouseSpeed * xRotation, mouseSpeed * yRotation, zRotation * rotationSpeed * (float)duration.count()));
		}

		xOffset = 0;
		yOffset = 0;
		zOffset = 0;

		xRotation = 0;
		yRotation = 0;
		zRotation = 0;

		Hedge::Renderer::BeginScene(primaryCamera);
		{
			glm::mat4 one = glm::mat4(1.0f);

			glm::mat4 translate = glm::translate(one, glm::vec3(-0.5f, 0.0f, 0.0f));
			Hedge::Renderer::Submit(vertexArray, translate);

			translate = glm::translate(one, glm::vec3(0.0f, -1.5f, 0.0f));
			Hedge::Renderer::Submit(vertexArray, translate);

			scene.OnUpdate(duration);
		}
		Hedge::Renderer::EndScene();
	}

	void OnGuiUpdate() override
	{
		ImGui::Begin("Camera");

		auto primaryCamera = scene.GetPrimaryCamera();
		primaryCamera.Get<Hedge::Camera>().CreateGuiControls();
		primaryCamera.Get<Hedge::Transform>().CreateGuiControls(true, true, false);

		ImGui::End();
	}

	void OnMessage(const Hedge::Message& message) override
	{
		if (message.GetMessageType() == Hedge::MessageType::WindowSize)
		{
			const Hedge::WindowSizeMessage& windowSizeMessage = dynamic_cast<const Hedge::WindowSizeMessage&>(message);

			if (windowSizeMessage.GetHeight() != viewportDesc.height
				|| windowSizeMessage.GetWidth() != viewportDesc.width)
			{
				std::dynamic_pointer_cast<Hedge::VulkanVertexArray>(vertexArray)->Resize(windowSizeMessage.GetWidth(), windowSizeMessage.GetHeight());

				viewportDesc.height = windowSizeMessage.GetHeight();
				viewportDesc.width = windowSizeMessage.GetWidth();
			}
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

		if (message.GetMessageType() == Hedge::MessageType::MouseScrolled)
		{
			const Hedge::MouseScrollMessage& mouseScrollMessage = dynamic_cast<const Hedge::MouseScrollMessage&>(message);

			yOffset += mouseScrollMessage.GetDistance();
		}
	}

private:
	Hedge::Scene scene;

	struct ViewportDesc
	{
		int x;
		int y;
		int width;
		int height;

		bool operator!=(const ViewportDesc& other)
		{
			return x != other.x || y != other.y || width != other.width || height != other.height;
		}
	};

	ViewportDesc viewportDesc;

	// Camera
	float aspectRatio;
	float cameraFOV = 56.0f;
	float cameraZoom = 1.0f;

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

	// Render settings
	bool wireframeMode;
	bool depthTest;
	bool faceCulling;
	bool blending;

	bool previousWireframeMode;
	bool previousDepthTest;
	bool previousFaceCulling;
	bool previousBlending;


	std::shared_ptr<Hedge::Shader> shader;
	std::shared_ptr<Hedge::VertexArray> vertexArray;
	std::shared_ptr<Hedge::VertexBuffer> vertexBuffer1;
	std::shared_ptr<Hedge::VertexBuffer> vertexBuffer2;
	std::shared_ptr<Hedge::VertexBuffer> vertexBuffer3;
	std::shared_ptr<Hedge::IndexBuffer> indexBuffer;
};

class VulkanTest : public Hedge::Application
{
public:
	VulkanTest(HINSTANCE hInstance) : Application(hInstance)
	{
		layers.PushOverlay(new ExampleOverlay("1st Example Overlay"));
		layers.Push(new VulkanTestLayer());
	}

	~VulkanTest()
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

	//Sandbox app(hInstance);
	VulkanTest app(hInstance);
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
