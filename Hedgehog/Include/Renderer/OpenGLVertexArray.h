#pragma once

#include <Renderer/VertexArray.h>
#include <Renderer/OpenGLShader.h>
#include <Model/Model.h>

#include <glad/glad.h>


namespace Hedge
{

class OpenGLVertexArray : public VertexArray
{
public:
	OpenGLVertexArray(const std::shared_ptr<Shader>& inputShader,
					  PrimitiveTopology primitiveTopology,
					  const std::vector<Hedge::TextureDescription>& textureDescriptions);
	virtual ~OpenGLVertexArray() override;

	virtual void Bind() override;
	virtual void Unbind() const override;

	virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
	virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
	virtual void AddTexture(TextureType type, int position, const std::shared_ptr<Texture>& texture) override;
	virtual void SetupGroups(const std::vector<VertexGroup>& groups) override;
	virtual void SetInstanceCount(unsigned int instanceCount) override { this->instanceCount = instanceCount; }

	virtual PrimitiveTopology GetPrimitiveTopology() const override { return primitiveTopology; }
	virtual const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const override { return vertexBuffers; }
	virtual const std::shared_ptr<IndexBuffer> GetIndexBuffer() const override { return indexBuffer; }
	virtual const std::shared_ptr<Shader> GetShader() const override { return shader; }
	virtual std::vector<std::pair<VertexGroup, float>>& GetGroups() override { return groups; }
	virtual unsigned int GetInstanceCount() const override { return instanceCount; }

private:
	unsigned int GetOpenGLBaseType(ShaderDataType type) const { return OpenGLBaseTypes[(int)type]; }


private:
	unsigned int rendererID = 0;
	std::shared_ptr<OpenGLShader> shader;
	PrimitiveTopology primitiveTopology;
	std::vector<TextureDescription> textureDescriptions;
	std::vector<std::shared_ptr<Texture>> textures;
	unsigned int vertexAttribIndex = 0;
	std::vector<std::shared_ptr<VertexBuffer>> vertexBuffers;
	std::shared_ptr<IndexBuffer> indexBuffer;
	std::vector<std::pair<VertexGroup, float>> groups;
	unsigned int instanceCount = 1;

	// Array of OpenGL base types corresponding to ShaderDataType
	// TODECIDE map, switch better?
	const unsigned int OpenGLBaseTypes[10] =
	{
		GL_NONE,
		GL_FLOAT,
		GL_FLOAT,
		GL_FLOAT,
		GL_FLOAT,
		GL_INT,
		GL_INT,
		GL_INT,
		GL_INT,
		GL_BOOL
	};
};

} // namespace Hedge
