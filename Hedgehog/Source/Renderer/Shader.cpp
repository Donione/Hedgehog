#include <Renderer/Renderer.h>
#include <Renderer/Shader.h>
#include <Renderer/DirectX12Shader.h>
#include <Renderer/OpenGLShader.h>


Shader* Shader::Create(const std::string& filePath)
{
	return Create(filePath, filePath);
}

Shader* Shader::Create(const std::string& vertexFilePath, const std::string& pixelFilePath)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLShader(vertexFilePath, pixelFilePath);

	case RendererAPI::API::DirectX12:
		return new DirectX12Shader(std::wstring(vertexFilePath.begin(), vertexFilePath.end()),
								   std::string("VSMain"),
								   std::wstring(pixelFilePath.begin(), pixelFilePath.end()),
								   std::string("PSMain"));

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}
