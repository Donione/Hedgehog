#include <Component/Mesh.h>

#include <Model/Model.h>

#include <fstream>
#include <unordered_map>


namespace Hedge
{

Mesh::Mesh(const std::string& modelFilename,
		   PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		   ConstantBufferDescription constBufferDesc,
		   const std::string& VSfilename, const std::string& PSfilename, const std::string& GSfilename,
		   const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	Model model;
	if (modelFilename.ends_with(".tri"))
	{
		model.LoadTri(modelFilename);
	}
	else if (modelFilename.ends_with(".obj"))
	{
		model.LoadObj(modelFilename);
	}

	CreateMesh(model.GetVertices(), model.GetSizeOfVertices(),
			   model.GetIndices(), model.GetNumberOfIndices(),
			   primitiveTopology, bufferLayout,
			   constBufferDesc,
			   VSfilename, PSfilename, GSfilename,
			   textureDescriptions,
			   {});
}

Mesh::Mesh(const float* vertices, unsigned int sizeOfVertices,
		   const unsigned int* indices, unsigned int numberOfIndices,
		   PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		   ConstantBufferDescription constBufferDesc,
		   const std::string& VSfilename, const std::string& PSfilename, const std::string& GSfilename,
		   const std::vector<Hedge::TextureDescription>& textureDescriptions,
		   const std::vector<VertexGroup>& groups)
{
	CreateMesh(vertices, sizeOfVertices,
			   indices, numberOfIndices,
			   primitiveTopology, bufferLayout,
			   constBufferDesc,
			   VSfilename, PSfilename, GSfilename,
			   textureDescriptions,
			   groups);
}

void Mesh::CreateMesh(const float* vertices, unsigned int sizeOfVertices,
					 const unsigned int* indices, unsigned int numberOfIndices,
					 PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
					 ConstantBufferDescription constBufferDesc,
					 const std::string& VSfilename, const std::string& PSfilename, const std::string& GSfilename,
					 const std::vector<Hedge::TextureDescription>& textureDescriptions,
					 const std::vector<VertexGroup>& groups)
{
	auto shader = std::shared_ptr<Shader>(Hedge::Shader::Create(VSfilename, PSfilename, GSfilename));
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
			std::shared_ptr<Texture> texture;
			texture.reset(Hedge::Texture2D::Create(textureDesc.filename));

			vertexArray->AddTexture(textureDesc.type, texturePosition[textureDesc.type]++, texture);
		}
	}

	auto vertexBuffer = std::shared_ptr<VertexBuffer>(VertexBuffer::Create(bufferLayout, vertices, sizeOfVertices));
	vertexArray->AddVertexBuffer(vertexBuffer);

	auto indexBuffer = std::shared_ptr<IndexBuffer>(Hedge::IndexBuffer::Create(indices, numberOfIndices));
	vertexArray->AddIndexBuffer(indexBuffer);

	vertexArray->SetupGroups(groups);
}

} // namespace Hedge
