#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>


namespace Hedge
{

enum class ModelType
{
	Tri,
	Obj,
	Unknown,
};

struct FaceVertex
{
	int vertex = -1;
	int texCoord = -1;
	int normal = -1;
};

struct Face
{
	FaceVertex v0;
	FaceVertex v1;
	FaceVertex v2;
};

class Model
{
public:
	Model() = default;
	~Model();

	void LoadTri(const std::string& filename);

	const float* const GetVertices();
	unsigned int GetSizeOfVertices() const;
	const unsigned int* const GetIndices();
	unsigned int GetNumberOfIndices() const;

private:
	ModelType type = ModelType::Unknown;

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> textureCoordinates;
	std::vector<glm::vec3> normals;

	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangets;

	std::vector<Face> faces;

	// Flat, heap allocated arrays of vertices and indices to be passed into the GPU (via Mesh)
	float* flatVertices = nullptr;
	unsigned int* flatIndices = nullptr;
};

} // namespace Hedge
