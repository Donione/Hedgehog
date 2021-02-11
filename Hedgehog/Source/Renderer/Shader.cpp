#include <Renderer/Renderer.h>
#include <Renderer/Shader.h>
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

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}
