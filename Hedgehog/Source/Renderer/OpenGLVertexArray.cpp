#include <Renderer/OpenGLVertexArray.h>
#include <Renderer/Buffer.h>


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

void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	this->Bind();
	vertexBuffer->Bind();
	vertexBuffers.push_back(vertexBuffer);

	unsigned int index = 0;

	const BufferLayout& layout = vertexBuffer->GetLayout();
	for (const auto& element : layout)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index,
							  GetShaderDataTypeCount(element.type),
							  GetOpenGLBaseType(element.type),
							  element.normalized ? GL_TRUE : GL_FALSE,
							  layout.GetStride(),
							  (const void*)element.offset);

		index++;
	}
}

void OpenGLVertexArray::AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	this->Bind();
	indexBuffer->Bind();
	indexBuffers.push_back(indexBuffer);
}
