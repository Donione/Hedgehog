#pragma once

#include <Renderer/VertexArray.h>
#include <Component/Transform.h>
#include <Model/Model.h>


namespace Hedge
{

class Mesh
{
public:
	Mesh() = default;

	// Create mesh by:
	//    loading model from a file
	//    loading shaders' source code from files
	//    [optionally] preparing textures from a description
	Mesh(const std::string& modelFilename,
		 PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		 ConstantBufferDescription constBufferDesc,
		 const std::string& VSfilename, const std::string& PSfilename, const std::string& GSfilename = "",
		 const std::vector<Hedge::TextureDescription>& textureDescriptions = {});

	// Create mesh by:
	//    using provided vertices and indices
	//    loading shaders' source code from files
	//    [optionally] preparing textures from a description
	Mesh(const float* vertices, unsigned int sizeOfVertices,
		 const unsigned int* indices, unsigned int numberOfIndices,
		 PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		 ConstantBufferDescription constBufferDesc,
		 const std::string& VSfilename, const std::string& PSfilename, const std::string& GSfilename = "",
		 const std::vector<Hedge::TextureDescription>& textureDescriptions = {},
		 const std::vector<VertexGroup>& groups = {});

	const std::shared_ptr<VertexArray>& Get() const { return vertexArray; }
	const std::shared_ptr<Shader> GetShader() const { return vertexArray->GetShader(); }

private:
	void CreateMesh(const float* vertices, unsigned int sizeOfVertices,
					const unsigned int* indices, unsigned int numberOfIndices,
					PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
					ConstantBufferDescription constBufferDesc,
					const std::string& VSfilename, const std::string& PSfilename, const std::string& GSfilename,
					const std::vector<Hedge::TextureDescription>& textureDescriptions,
					const std::vector<VertexGroup>& groups);


public:
	bool enabled = true;

private:
	std::shared_ptr<VertexArray> vertexArray;
};

} // namespace Hedge
