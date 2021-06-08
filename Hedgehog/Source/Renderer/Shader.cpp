#include <Renderer/Shader.h>

#include <Renderer/Renderer.h>
#include <Renderer/DirectX12Shader.h>
#include <Renderer/OpenGLShader.h>
#include <Renderer/VulkanShader.h>


namespace Hedge
{

Shader* Shader::Create(const std::string& filePath)
{
	return Create(filePath, filePath, filePath);
}

Shader* Shader::Create(const std::string& vertexFilePath,
					   const std::string& pixelFilePath,
					   const std::string& geometryFilePath)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLShader(vertexFilePath, pixelFilePath, geometryFilePath);

	case RendererAPI::API::DirectX12:
		return new DirectX12Shader(std::wstring(vertexFilePath.begin(), vertexFilePath.end()),
								   std::string("VSMain"),
								   std::wstring(pixelFilePath.begin(), pixelFilePath.end()),
								   std::string("PSMain"),
								   std::wstring(geometryFilePath.begin(), geometryFilePath.end()),
								   std::string("GSMain"));

	case RendererAPI::API::Vulkan:
		return new VulkanShader(vertexFilePath, pixelFilePath, geometryFilePath);

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}

} // namespace Hedge
