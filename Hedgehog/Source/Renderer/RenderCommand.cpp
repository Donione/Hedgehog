#include <Renderer/RenderCommand.h>

#include <Renderer/OpenGLRendererAPI.h>


RendererAPI* RenderCommand::rendererAPI = new OpenGLRendererAPI();
void RenderCommand::Init(RenderContext* renderContext)
{
	if (rendererAPI)
	{
		rendererAPI->Init(renderContext);
	}
}
