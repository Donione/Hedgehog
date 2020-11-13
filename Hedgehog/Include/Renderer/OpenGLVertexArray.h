#pragma once

#include <Renderer/VertexArray.h>


class OpenGLVertexArray : public VertexArray
{
public:
	OpenGLVertexArray();
	virtual ~OpenGLVertexArray();

	virtual void Bind() const override;
	virtual void Unbind() const override;

private:
	unsigned int rendererID = 0;
};
