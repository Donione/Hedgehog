#pragma once

#include <vector>
#include <string>
#include <map>

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

	int faceNormal = -1;
};

struct Face
{
	FaceVertex v[3];
};

class Model
{
public:
	Model() = default;
	~Model();

	void LoadTri(const std::string& filename);
	void LoadObj(const std::string& filename);

	const float* const GetVertices() const { return flatVertices; }
	unsigned int GetSizeOfVertices() const;
	const unsigned int* const GetIndices() const { return flatIndices; }
	unsigned int GetNumberOfIndices() const;

private:
	void CalculateFaceNormals();
	void MapIndices();
	void CalculateTangents();

	void CreateFlatArraysTri();
	void CreateFlatArraysObj();


private:
	ModelType type = ModelType::Unknown;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textureCoordinates;
	std::vector<glm::vec3> normals;

	std::vector<glm::vec3> faceNormals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangets;

	std::vector<Face> faces;

	struct cmpByFaceVertex {
		bool operator()(const FaceVertex& first, const FaceVertex& second) const {
			if (first.vertex != second.vertex)
			{
				return first.vertex < second.vertex;
			}
			else
			{
				if (first.texCoord != second.texCoord)
				{
					return first.texCoord < second.texCoord;
				}
				else
				{
					return first.faceNormal < second.faceNormal;
				}
			}
		}
	};
	std::map<FaceVertex, unsigned int, cmpByFaceVertex> indices;

	// Flat, heap allocated arrays of vertices and indices to be passed into the GPU (via Mesh)
	float* flatVertices = nullptr;
	unsigned int* flatIndices = nullptr;
};

} // namespace Hedge
