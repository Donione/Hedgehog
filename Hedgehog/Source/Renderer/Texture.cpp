#include <Renderer/Texture.h>
#include <Renderer/Renderer.h>

#include <Renderer/OpenGLTexture.h>


Texture2D* Texture2D::Create(const std::string& filename)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		return new OpenGLTexture2D(filename);

	case RendererAPI::API::None:
		return nullptr;

	default:
		return nullptr;
	}
}
