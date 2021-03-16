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
	//    [optionally] loading texture from a file
	Mesh(const std::string& modelFilename, PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		 const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		 const std::string& textureFilename = "");

	// Create mesh by:
	//    using provided vertices and indices
	//    loading shaders' source code from files
	//    [optionally] loading texture from a file
	Mesh(const float* vertices, unsigned int sizeOfVertices,
		 const unsigned int* indices, unsigned int numberOfIndices,
		 PrimitiveTopology primitiveTopology, BufferLayout bufferLayout,
		 const std::string& VSfilename, const std::string& PSfilename, ConstantBufferDescription constBufferDesc,
		 const std::string& textureFilename = "");

	const std::shared_ptr<VertexArray>& Get() const { return vertexArray; }
	const std::shared_ptr<Shader> GetShader() const { return shader; }

public:
	// TODO transform will go away from the mesh component and will be handled by ECS
	Transform transform;


private:
	std::shared_ptr<VertexArray> vertexArray;

	std::shared_ptr<Shader> shader;
	std::shared_ptr<VertexBuffer> vertexBuffer;
	std::shared_ptr<IndexBuffer> indexBuffer;
	std::shared_ptr<Texture> texture;
};

} // namespace Hedge
