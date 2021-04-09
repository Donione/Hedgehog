#include <Component/Mesh.h>

#include <Model/Model.h>

#include <fstream>
#include <unordered_map>


namespace Hedge
{

Mesh::Mesh(const std::string& modelFilename, PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		   const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		   const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	Model model;
	model.LoadTri(modelFilename);

	CreateMesh(model.GetVertices(), model.GetSizeOfVertices(),
			   model.GetIndices(), model.GetNumberOfIndices(),
			   primitiveTopology, bufferLayout,
			   VSfilename, PSfilename, constBufferDesc,
			   textureDescriptions);
}

Mesh::Mesh(const float* vertices, unsigned int sizeOfVertices,
		   const unsigned int* indices, unsigned int numberOfIndices,
		   PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		   const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		   const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	CreateMesh(vertices, sizeOfVertices,
			  indices, numberOfIndices,
			  primitiveTopology, bufferLayout,
			  VSfilename, PSfilename, constBufferDesc,
			  textureDescriptions);
}

void Mesh::CreateMesh(const float* vertices, unsigned int sizeOfVertices,
					 const unsigned int* indices, unsigned int numberOfIndices,
					 PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
					 const std::string& VSfilename, const std::string& PSfilename,
					 ConstantBufferDescription constBufferDesc,
					 const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	auto shader = std::shared_ptr<Shader>(Hedge::Shader::Create(VSfilename, PSfilename));
	shader->SetupConstantBuffers(constBufferDesc);

	vertexArray.reset(Hedge::VertexArray::Create(shader, primitiveTopology, bufferLayout, textureDescriptions));

	std::unordered_map<TextureType, int> texturePosition = 
	{
		{ TextureType::Diffuse, 0 },
		{ TextureType::Specular, 0 },
		{ TextureType::Normal, 0 },
		{ TextureType::Generic, 0 },
	};
	for (auto& textureDesc : textureDescriptions)
	{
		if (!textureDesc.filename.empty())
		{
			//auto texture = std::make_shared<Texture>(Hedge::Texture2D::Create(textureDesc.filename));
			std::shared_ptr<Texture> texture;
			texture.reset(Hedge::Texture2D::Create(textureDesc.filename));

			//textures.push_back(texture);
			vertexArray->AddTexture(textureDesc.type, texturePosition[textureDesc.type]++, texture);
		}
	}

	auto vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::Create(primitiveTopology, bufferLayout, vertices, sizeOfVertices));
	vertexArray->AddVertexBuffer(vertexBuffer);

	auto indexBuffer = std::shared_ptr<IndexBuffer>(Hedge::IndexBuffer::Create(indices, numberOfIndices));
	vertexArray->AddIndexBuffer(indexBuffer);
}

} // namespace Hedge
