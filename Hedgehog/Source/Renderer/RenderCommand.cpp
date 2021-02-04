#include <Renderer/RenderCommand.h>

#include <Renderer/Renderer.h>
#include <Renderer/OpenGLRendererAPI.h>


void RenderCommand::Init(RenderContext* renderContext)
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		rendererAPI = new OpenGLRendererAPI();
		break;
	case RendererAPI::API::None:
		rendererAPI = nullptr;
		break;

	default:
		rendererAPI = nullptr;
		break;
	}

	if (rendererAPI)
	{
		rendererAPI->Init(renderContext);
	}
}
