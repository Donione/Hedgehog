#include <Renderer/OpenGLVertexArray.h>


namespace Hedge
{

OpenGLVertexArray::OpenGLVertexArray(const std::shared_ptr<Shader>& inputShader,
									 PrimitiveTopology primitiveTopology,
									 const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	this->shader = std::dynamic_pointer_cast<OpenGLShader>(inputShader);
	this->primitiveTopology = primitiveTopology;
	this->textureDescriptions = textureDescriptions;
	textures.resize(textureDescriptions.size());

	glCreateVertexArrays(1, &rendererID);
	// Unbind the vertex array so a vertex or index buffer intended for a different VA isn't bound to this one upon creation.
	glBindVertexArray(0);
}

OpenGLVertexArray::~OpenGLVertexArray()
{
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &rendererID);
}

void OpenGLVertexArray::Bind()
{
	// TODO check if we have all the textures, a vertex buffer and an index buffer

	glBindVertexArray(rendererID);
	shader->Bind();

	unsigned int slot = 0;
	for (auto& texture : textures)
	{
		if (texture) texture->Bind(slot);
		slot++;
	}
}

void OpenGLVertexArray::Unbind() const
{
	glBindVertexArray(0);
	shader->Unbind();
}

void OpenGLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
{
	this->Bind();
	vertexBuffer->Bind();
	vertexBuffers.push_back(vertexBuffer);

	const BufferLayout& layout = vertexBuffer->GetLayout();
	for (const auto& element : layout)
	{
		glEnableVertexAttribArray(vertexAttribIndex);
		glVertexAttribPointer(vertexAttribIndex,
							  GetShaderDataTypeCount(element.type),
							  GetOpenGLBaseType(element.type),
							  element.normalized ? GL_TRUE : GL_FALSE,
							  layout.GetStride(),
							  (const void*)element.offset);
		if (element.instanceDataStep == -1)
		{
			glVertexAttribDivisor(vertexAttribIndex, 0);
		}
		else if (element.instanceDataStep == 0)
		{
			// OpenGL doesn't support zero divisor instance data rate, afaik
			// TODO find out if there's an externsion or something for it
			assert(element.instanceDataStep != 0);
		}
		else
		{
			glVertexAttribDivisor(vertexAttribIndex, (GLuint)element.instanceDataStep);
		}

		vertexAttribIndex++;
	}

	this->Unbind();
}

void OpenGLVertexArray::AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	this->Bind();
	indexBuffer->Bind();
	this->indexBuffer = indexBuffer;
	this->Unbind();
}

void OpenGLVertexArray::AddTexture(TextureType type, int position, const std::shared_ptr<Texture>& texture)
{
	auto indices = FindIndices(type, textureDescriptions);
	assert(indices.size() >= (position + 1));
	int index = indices[position];
	textures[index] = texture;

	std::string name;
	switch (type)
	{
	case TextureType::Diffuse: name = "t_diffuse[" + std::to_string(position) + "]"; break;
	case TextureType::Specular: name = "t_specular[" + std::to_string(position) + "]"; break;
	case TextureType::Normal: name = "t_normal[" + std::to_string(position) + "]"; break;
	case TextureType::Generic: name = "t_texture[" + std::to_string(position) + "]"; break;
	default: assert(false); break;
	}

	shader->UploadConstant(name, index);
}

void OpenGLVertexArray::SetupGroups(const std::vector<VertexGroup>& groups)
{
	for (auto& group : groups)
	{
		this->groups.emplace_back(group, 0.0f);
	}
}

} // namespace Hedge
