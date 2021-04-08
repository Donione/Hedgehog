#include <Component/Mesh.h>

#include <fstream>
#include <unordered_map>


namespace Hedge
{

void loadModel(const std::string& filename,
			   long long int& numberOfVertices, float*& vertices,
			   long long int& numberOfIndices, unsigned int*& indices)
{
	std::ifstream in(filename);
	char buffer[256];

	// header, just discard
	in.getline(buffer, 256);

	// number of vertices
	std::string text;
	in >> text >> numberOfVertices;
	in >> text >> numberOfIndices;

	vertices = new float[numberOfVertices * 3 * 2];
	indices = new unsigned int[numberOfIndices * 3];

	for (int vertex = 0; vertex < numberOfVertices; vertex++)
	{
		in >> vertices[vertex * 6 + 0] >> vertices[vertex * 6 + 1] >> vertices[vertex * 6 + 2];
		vertices[vertex * 6 + 3] = vertices[vertex * 6 + 4] = vertices[vertex * 6 + 5] = 0.0;
	}

	for (int index = 0; index < numberOfIndices; index++)
	{
		in >> indices[index * 3 + 0] >> indices[index * 3 + 1] >> indices[index * 3 + 2];

		// calculate the triangle normal
		glm::vec3 v0(vertices[indices[index * 3 + 0] * 6 + 0], vertices[indices[index * 3 + 0] * 6 + 1], vertices[indices[index * 3 + 0] * 6 + 2]);
		glm::vec3 v1(vertices[indices[index * 3 + 1] * 6 + 0], vertices[indices[index * 3 + 1] * 6 + 1], vertices[indices[index * 3 + 1] * 6 + 2]);
		glm::vec3 v2(vertices[indices[index * 3 + 2] * 6 + 0], vertices[indices[index * 3 + 2] * 6 + 1], vertices[indices[index * 3 + 2] * 6 + 2]);

		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

		// to estimate vertex normals, accumulate triangle normals in vertices (and normalizes them when all triangles were processed or in a shader)
		vertices[indices[index * 3 + 0] * 6 + 3] += normal.x;
		vertices[indices[index * 3 + 0] * 6 + 4] += normal.y;
		vertices[indices[index * 3 + 0] * 6 + 5] += normal.z;

		vertices[indices[index * 3 + 1] * 6 + 3] += normal.x;
		vertices[indices[index * 3 + 1] * 6 + 4] += normal.y;
		vertices[indices[index * 3 + 1] * 6 + 5] += normal.z;

		vertices[indices[index * 3 + 2] * 6 + 3] += normal.x;
		vertices[indices[index * 3 + 2] * 6 + 4] += normal.y;
		vertices[indices[index * 3 + 2] * 6 + 5] += normal.z;
	}
}


Mesh::Mesh(const std::string& modelFilename, PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		   const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		   const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	long long int numberOfVertices;
	long long int numberOfIndices;
	float* vertices = NULL;
	unsigned int* indices = NULL;
	loadModel(modelFilename, numberOfVertices, vertices, numberOfIndices, indices);

	CrateMesh(vertices, sizeof(float) * 6U * (unsigned int)numberOfVertices,
			  indices, 3 * (unsigned int)numberOfIndices,
			  primitiveTopology, bufferLayout,
			  VSfilename, PSfilename, constBufferDesc,
			  textureDescriptions);
	
	delete vertices;
	delete indices;
}

Mesh::Mesh(const float* vertices, unsigned int sizeOfVertices,
		   const unsigned int* indices, unsigned int numberOfIndices,
		   PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		   const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		   const std::vector<Hedge::TextureDescription>& textureDescriptions)
{
	CrateMesh(vertices, sizeOfVertices,
			  indices, numberOfIndices,
			  primitiveTopology, bufferLayout,
			  VSfilename, PSfilename, constBufferDesc,
			  textureDescriptions);
}

void Mesh::CrateMesh(const float* vertices, unsigned int sizeOfVertices,
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
