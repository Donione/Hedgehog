#pragma once

enum class RendererAPI
{
	None = 0,
	OpenGL = 1
};

class Renderer
{
public:
	static RendererAPI GetAPI() { return rendererAPI; }

private:
	static RendererAPI rendererAPI;
};
