#include <Renderer/RenderCommand.h>

#include <Renderer/Renderer.h>
#include <Renderer/OpenGLRendererAPI.h>
#include <Renderer/DirectX12RendererAPI.h>
#include <Renderer/VulkanRendererAPI.h>


namespace Hedge
{

void RenderCommand::Create()
{
	switch (Renderer::GetAPI())
	{
	case RendererAPI::API::OpenGL:
		rendererAPI = new OpenGLRendererAPI();
		break;

	case RendererAPI::API::DirectX12:
		rendererAPI = new DirectX12RendererAPI();
		break;

	case RendererAPI::API::Vulkan:
		rendererAPI = new VulkanRendererAPI();
		break;

	case RendererAPI::API::None:
		rendererAPI = nullptr;
		break;

	default:
		rendererAPI = nullptr;
		break;
	}

	assert(rendererAPI);
}

void RenderCommand::Init(RenderContext* renderContext)
{
	if (rendererAPI)
	{
		rendererAPI->Init(renderContext);
	}
}

} // namespace Hedge
