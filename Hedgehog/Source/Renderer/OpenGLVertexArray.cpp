#include <Renderer/OpenGLVertexArray.h>

#include <glad/glad.h>


OpenGLVertexArray::OpenGLVertexArray()
{
	glCreateVertexArrays(1, &rendererID);
}

OpenGLVertexArray::~OpenGLVertexArray()
{
	glDeleteVertexArrays(1, &rendererID);
}

void OpenGLVertexArray::Bind() const
{
	glBindVertexArray(rendererID);
}

void OpenGLVertexArray::Unbind() const
{
	glBindVertexArray(0);
}
