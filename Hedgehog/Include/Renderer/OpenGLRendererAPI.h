#pragma once

#include <Renderer/RendererAPI.h>
#include <Renderer/OpenGLContext.h>

#include <glad/glad.h>


namespace Hedge
{

class OpenGLRendererAPI : public RendererAPI
{
public:
	virtual void Init(RenderContext* renderContext) override;

	virtual void SetWireframeMode(bool enable) override;
	virtual void SetDepthTest(bool enable) override;
	virtual void SetFaceCulling(bool enable) override;
	virtual void SetBlending(bool enable) override;

	virtual bool GetWireframeMode() const override { return wireframeMode; }
	virtual bool GetDepthTest() const override { return depthTest; }
	virtual bool GetFaceCulling() const override { return faceCulling; }
	virtual bool GetBlending() const override { return blending; }

	virtual void Resize(int width, int height, bool fillViewport = true) override;
	virtual void SetViewport(int x, int y, int width, int height) override;
	virtual void SetScissor(int x, int y, int width, int height) override;

	virtual void SetClearColor(const glm::vec4& color) override;
	virtual void End() override { /* do nothing */ }
	virtual void Begin() override { /* do nothing */ }
	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

private:
	GLenum GetPipelinePrimitiveTopology(PrimitiveTopology topology) const { return pipelinePrimitiveTopologies[(int)topology]; }


private:
	OpenGLContext* renderContext;

	bool wireframeMode = false; // in OpenGL it is disabled by default
	bool depthTest = false; // in OpenGL it is disabled by default
	bool faceCulling = false; // in OpenGL it is disabled by default
	bool blending = false; // in OpenGL it is disabled by default

	const GLenum pipelinePrimitiveTopologies[4]
	{
		GL_NONE,
		GL_POINTS,
		GL_LINES,
		GL_TRIANGLES,
	};
};

} // namespace Hedge
