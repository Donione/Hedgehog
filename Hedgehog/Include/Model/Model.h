#pragma once

#include <Renderer/Texture.h>
#include <Animation/Animation.h>

#include <vector>
#include <string>
#include <map>
#include <sstream>

#include <pugixml.hpp>

#include <glm/glm.hpp>


namespace Hedge
{

enum class ModelType
{
	Tri,
	Obj,
	Dae,
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

struct VertexGroup
{
	bool enabled = true;
	std::string name;
	unsigned int startIndex = 0;
	unsigned int endIndex = 0;
	glm::vec3 center{ 0.0f };
};

struct Material
{
	std::string name;
	int textureSlot;
	std::string diffuseFilename;
	std::string normalFilename;
};

struct SegmentIDs
{
	int ID[4];
};

struct SegmentWeightIndices
{
	int i[4];
};


class Model
{
public:
	Model() = default;
	~Model();

	void LoadTri(const std::string& filename);
	void LoadObj(const std::string& filename);
	void LoadDae(const std::string& filename);

	const float* const GetVertices() const { return flatVertices.data(); }
	unsigned int GetSizeOfVertices() const;
	const unsigned int* const GetIndices() const { return flatIndices.data(); }
	unsigned int GetNumberOfIndices() const;
	const std::vector<Hedge::TextureDescription>& GetTextureDescription() const { return textureDescription; }
	Animation* GetAnimation() { return &animation; }

	const float* const GetTBNVertices() const { return flatTBNVertices.data(); }
	unsigned int GetSizeOfTBNVertices() const;
	const unsigned int* const GetTBNIndices() const { return flatTBNIndices.data(); }
	unsigned int GetNumberOfTBNIndices() const;

	const std::vector<VertexGroup>& GetGroups() const { return groups; }

private:
	void LoadMtl(const std::string& filename);
	void CreateTextureDescription();

	void CalculateFaceNormals();
	void MapIndices();
	void CalculateTangents();

	void CreateFlatArraysTri();
	void CreateFlatArraysObj();

	void CreateTBNFlatArraysObj();

	void CalculateCenters();

	template <typename T>
	std::vector<T> CreateSource(const pugi::xml_node& node, const std::string& sourceName)
	{
		std::string xpath = "source[contains(@id, '" + sourceName;
		if (typeid(T) == typeid(float))
		{
			xpath += "')]/float_array";
		}
		else if (typeid(T) == typeid(std::string))
		{
			xpath += "')]/Name_array";
		}

		auto arrayNode = node.select_node(xpath.c_str()).node();
		int arrayCount = arrayNode.attribute("count").as_int();
		std::stringstream arrayStream(arrayNode.first_child().value());

		std::vector<T> result;
		for (int i = 0; i < arrayCount; i++)
		{
			T value;
			arrayStream >> value;
			result.push_back(value);
		}

		return result;
	}


private:
	ModelType type = ModelType::Unknown;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textureCoordinates;
	std::vector<glm::vec3> normals;

	std::vector<std::string> segmentNames;
	std::map<std::string, int> segmentMap;
	// pairs of segement and its parent
	std::vector<std::pair<Segment, int>> segments;
	std::vector<float> segmentWeights;

	std::vector<SegmentIDs> segmentIDs;
	std::vector<SegmentWeightIndices> segmentWeightIndices;

	std::vector<glm::vec3> faceNormals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec3> bitangents;

	std::vector<Face> faces;

	std::vector<VertexGroup> groups;
	std::map<std::string, Material> materials;

	std::vector<Hedge::TextureDescription> textureDescription;

	Animation animation;

	struct cmpByFaceVertex
	{
		bool operator()(const FaceVertex& first, const FaceVertex& second) const
		{
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
