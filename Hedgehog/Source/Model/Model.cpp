#include <Model/Model.h>

#include <fstream>


namespace Hedge
{

void Model::LoadTri(const std::string& filename)
{
	type = ModelType::Tri;

	std::ifstream in(filename);
	char buffer[256];

	// header, just discard
	in.getline(buffer, 256);

	std::string text;
	long long int numberOfVertices = -1;
	long long int numberOFaces = -1;

	in >> text >> numberOfVertices;
	in >> text >> numberOFaces;

	glm::vec3 vertex{ 0.0f };
	for (int i = 0; i < numberOfVertices; i++)
	{
		in >> vertex.x >> vertex.y >> vertex.z;
		vertices.push_back(vertex);
		normals.push_back(glm::vec3(0.0f));
	}

	unsigned int v0, v1, v2;
	for (int i = 0; i < numberOFaces; i++)
	{
		in >> v0 >> v1 >> v2;

		faces.push_back({ {(int)v0, -1, (int)v0},
						  {(int)v1, -1, (int)v1},
						  {(int)v2, -1, (int)v2} });

		// calculate the face normal
		glm::vec3 normal = glm::normalize(glm::cross(vertices[v1] - vertices[v0], vertices[v2] - vertices[v0]));

		// to estimate vertex normals, accumulate face normals in vertices
		// (and normalizes them when all triangles were processed or in a shader)
		normals[v0] += normal;
		normals[v1] += normal;
		normals[v2] += normal;
	}
}

const float* const Model::GetVertices()
{
	if (!flatVertices)
	{
		if (type == ModelType::Tri)
		{
			auto numberOfVertices = vertices.size();
			flatVertices = new float[numberOfVertices * (3 + 3)];

			for (int i = 0; i < numberOfVertices; i++)
			{
				flatVertices[i * 6 + 0] = vertices[i].x;
				flatVertices[i * 6 + 1] = vertices[i].y;
				flatVertices[i * 6 + 2] = vertices[i].z;

				flatVertices[i * 6 + 3] = normals[i].x;
				flatVertices[i * 6 + 4] = normals[i].y;
				flatVertices[i * 6 + 5] = normals[i].z;
			}
		}
	}

	return flatVertices;
}

unsigned int Model::GetSizeOfVertices() const
{
	if (type == ModelType::Tri)
	{
		// Tri models don't have texture coordinates, so no tangents and bitangents either
		// Each vertex contains only its position and normal
		return (unsigned int)(sizeof(float) * vertices.size()) * (3 + 3);
	}

	return 0;
}

const unsigned int* const Model::GetIndices()
{
	if (!flatIndices)
	{
		auto numberOfFaces = faces.size();
		flatIndices = new unsigned int[numberOfFaces * 3];

		for (int i = 0; i < numberOfFaces; i++)
		{
			flatIndices[i * 3 + 0] = (unsigned int)faces[i].v0.vertex;
			flatIndices[i * 3 + 1] = (unsigned int)faces[i].v1.vertex;
			flatIndices[i * 3 + 2] = (unsigned int)faces[i].v2.vertex;
		}
	}

	return flatIndices;
}

unsigned int Model::GetNumberOfIndices() const
{
	return (unsigned int)faces.size() * 3u;
}

Model::~Model()
{
	if (flatVertices) delete flatVertices;
	if (flatIndices) delete flatIndices;
}

} // namespace Hedge
