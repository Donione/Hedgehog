#include <Renderer/OpenGLRendererAPI.h>

#include <glad/glad.h>


void OpenGLRendererAPI::Init(RenderContext* renderContext)
{
	this->renderContext = dynamic_cast<OpenGLContext*>(renderContext);
}

void OpenGLRendererAPI::SetWireframeMode(bool enable)
{
	if (enable != wireframeMode)
	{
		wireframeMode = enable;

		if (wireframeMode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

void OpenGLRendererAPI::SetDepthTest(bool enable)
{
	if (enable != depthTest)
	{
		depthTest = enable;

		if (depthTest)
		{
			glDepthFunc(GL_LEQUAL); // TODO make this setable by API
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}
}

void OpenGLRendererAPI::SetFaceCulling(bool enable)
{
	if (enable != faceCulling)
	{
		faceCulling = enable;

		if (faceCulling)
		{
			glCullFace(GL_BACK); // default is GL_BACK TODO make this setable by API
			glFrontFace(GL_CCW); // default is CCW TODO make this setable by API
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
	}
}

void OpenGLRendererAPI::SetBlending(bool enable)
{
	if (enable != blending)
	{
		blending = enable;

		if (blending)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // default is GL_ONE, GL_ZERO TODO make this setable by API
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}
}

void OpenGLRendererAPI::SetViewport(int width, int height)
{
	glViewport(0, 0, width, height);
}

void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::BeginFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::EndFrame()
{
	renderContext->SwapBuffers();
}

void OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
{
	vertexArray->Bind();
	glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer().at(0)->GetCount(), GL_UNSIGNED_INT, nullptr);
	vertexArray->Unbind();
}
