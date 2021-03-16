#include <Component/Mesh.h>

#include <fstream>


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

		// get the triangle normal
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
		   const std::string& textureFilename)
{
	long long int numberOfVertices;
	long long int numberOfIndices;
	float* vertices = NULL;
	unsigned int* indices = NULL;
	loadModel(modelFilename, numberOfVertices, vertices, numberOfIndices, indices);

	shader.reset(Hedge::Shader::Create(VSfilename, PSfilename));
	shader->SetupConstantBuffers(constBufferDesc);

	if (!textureFilename.empty())
	{
		texture.reset(Hedge::Texture2D::Create(textureFilename));
	}

	vertexArray.reset(Hedge::VertexArray::Create(shader, primitiveTopology, bufferLayout, texture));

	vertexBuffer.reset(Hedge::VertexBuffer::Create(primitiveTopology, bufferLayout, vertices, sizeof(float) * 6 * (int)numberOfVertices));
	delete vertices;
	vertexArray->AddVertexBuffer(vertexBuffer);

	indexBuffer.reset(Hedge::IndexBuffer::Create(indices, 3 * (int)numberOfIndices));
	delete indices;
	vertexArray->AddIndexBuffer(indexBuffer);
}

Mesh::Mesh(const float* vertices, unsigned int sizeOfVertices,
		   const unsigned int* indices, unsigned int numberOfIndices,
		   PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		   const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		   const std::string& textureFilename)
{
	shader.reset(Hedge::Shader::Create(VSfilename, PSfilename));
	shader->SetupConstantBuffers(constBufferDesc);

	if (!textureFilename.empty())
	{
		texture.reset(Hedge::Texture2D::Create(textureFilename));
	}

	vertexArray.reset(Hedge::VertexArray::Create(shader, primitiveTopology, bufferLayout, texture));

	vertexBuffer.reset(Hedge::VertexBuffer::Create(primitiveTopology, bufferLayout, vertices, sizeOfVertices));
	vertexArray->AddVertexBuffer(vertexBuffer);

	indexBuffer.reset(Hedge::IndexBuffer::Create(indices, numberOfIndices));
	vertexArray->AddIndexBuffer(indexBuffer);
}

} // namespace Hedge
