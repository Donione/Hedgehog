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
	int group = 0;
	int smoothingGroup = 0;
	int material = -1;

	int vertex = -1;
	int texCoord = -1;
	int normal = -1;

	int tangent = 0;
	int faceNormal = -1;
};

struct Face
{
	FaceVertex v[3];
};

struct Material
{
	std::string name;
	int textureSlot;
	std::string diffuseFilename;
	std::string normalFilename;
};


class Model
{
public:
	Model() = default;
	~Model();

	void LoadTri(const std::string& filename);
	void LoadObj(const std::string& filename);

	const float* const GetVertices() const { return flatVertices.data(); }
	unsigned int GetSizeOfVertices() const;
	const unsigned int* const GetIndices() const { return flatIndices.data(); }
	unsigned int GetNumberOfIndices() const;

	const float* const GetTBNVertices() const { return flatTBNVertices.data(); }
	unsigned int GetSizeOfTBNVertices() const;
	const unsigned int* const GetTBNIndices() const { return flatTBNIndices.data(); }
	unsigned int GetNumberOfTBNIndices() const;

private:
	void LoadMtl(const std::string& filename);

	void CalculateFaceNormals();
	void MapIndices();
	void CalculateTangents();

	void CreateFlatArraysTri();
	void CreateFlatArraysObj();

	void CreateTBNFlatArraysObj();


private:
	ModelType type = ModelType::Unknown;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textureCoordinates;
	std::vector<glm::vec3> normals;

	std::vector<glm::vec3> faceNormals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;

	std::vector<Face> faces;

	std::map<std::string, int> groups;
	std::map<std::string, Material> materials;

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
					if (first.normal != second.normal)
					{
						return first.normal < second.normal;
					}
					else
					{
						return first.tangent < second.tangent;
					}
				}
			}
		}
	};
	std::map<FaceVertex, unsigned int, cmpByFaceVertex> indices;

	struct cmpByGroup
	{
		bool operator()(const FaceVertex& a, const FaceVertex& b) const
		{
			if (a.group != b.group)
			{
				return a.group < b.group;
			}
			else
			{
				return a.vertex < b.vertex;
			}
		}
	};
	std::map<FaceVertex, int, cmpByGroup> groupIndices;

	// Flat arrays of vertices and indices to be passed into the GPU (via Mesh)
	std::vector<float> flatVertices;
	std::vector<unsigned int> flatIndices;

	// Flat arrays of vertices and indices of normal, tangent and bitangent lines to be passed into the GPU (via Mesh)
	std::vector<float> flatTBNVertices;
	std::vector<unsigned int> flatTBNIndices;
};

} // namespace Hedge
