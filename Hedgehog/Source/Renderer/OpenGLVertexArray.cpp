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

void OpenGLVertexArray::Bind() const
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

	this->Unbind();
}

void OpenGLVertexArray::AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
{
	this->Bind();
	indexBuffer->Bind();
	indexBuffers.push_back(indexBuffer);
	this->Unbind();
}

void OpenGLVertexArray::AddTexture(TextureType type, const std::shared_ptr<Texture>& texture)
{
	// TODO warn if adding a single texture when there are multiple textures of the same type described
	AddTexture(type, 0, texture);
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

void OpenGLVertexArray::AddTexture(TextureType type, const std::vector<std::shared_ptr<Texture>>& textures)
{
	auto indices = FindIndices(type, textureDescriptions);
	assert(indices.size() == textures.size());

	for (int i = 0; i < textures.size(); i++)
	{
		AddTexture(type, i, textures[i]);
	}
}

void OpenGLVertexArray::SetupGroups(const std::vector<VertexGroup>& groups)
{
	for (auto& group : groups)
	{
		this->groups.emplace_back(group, 0.0f);
	}
}

} // namespace Hedge
