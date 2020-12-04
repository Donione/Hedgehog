#pragma once

#include <Renderer/RendererAPI.h>

class Renderer
{
public:

	static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
};
