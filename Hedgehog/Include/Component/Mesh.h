#pragma once

#include <Renderer/VertexArray.h>
#include <Component/Transform.h>


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
	Mesh(const std::string& modelFilename, PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		 const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		 const std::vector<Hedge::TextureDescription>& textureDescriptions = {});

	// Create mesh by:
	//    using provided vertices and indices
	//    loading shaders' source code from files
	//    [optionally] preparing textures from a description
	Mesh(const float* vertices, unsigned int sizeOfVertices,
		 const unsigned int* indices, unsigned int numberOfIndices,
		 PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		 const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		 const std::vector<Hedge::TextureDescription>& textureDescriptions = {});

	const std::shared_ptr<VertexArray>& Get() const { return vertexArray; }
	const std::shared_ptr<Shader> GetShader() const { return vertexArray->GetShader(); }

private:
	void CrateMesh(const float* vertices, unsigned int sizeOfVertices,
				   const unsigned int* indices, unsigned int numberOfIndices,
				   PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
				   const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
				   const std::vector<Hedge::TextureDescription>& textureDescriptions);


public:
	bool enabled = true;

private:
	std::shared_ptr<VertexArray> vertexArray;
};

} // namespace Hedge
