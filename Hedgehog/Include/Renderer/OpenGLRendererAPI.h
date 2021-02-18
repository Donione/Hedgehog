#pragma once

#include <Renderer/RendererAPI.h>
#include <Renderer/OpenGLContext.h>


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

	virtual void SetViewport(int width, int height) override;

	virtual void SetClearColor(const glm::vec4& color) override;
	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;

private:
	OpenGLContext* renderContext;

	bool wireframeMode = false; // in OpenGL it is disabled by default
	bool depthTest = false; // in OpenGL it is disabled by default
	bool faceCulling = false; // in OpenGL it is disabled by default
	bool blending = false; // in OpenGL it is disabled by default
};
