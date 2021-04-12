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
		positions.push_back(vertex);
		normals.push_back(glm::vec3(0.0f));
	}

	unsigned int v0, v1, v2;
	for (int i = 0; i < numberOFaces; i++)
	{
		in >> v0 >> v1 >> v2;

		faces.push_back({ { {(int)v0, -1, (int)v0},
						    {(int)v1, -1, (int)v1},
						    {(int)v2, -1, (int)v2} } });

		// calculate the face normal
		glm::vec3 normal = glm::normalize(glm::cross(positions[v1] - positions[v0], positions[v2] - positions[v0]));

		// to estimate vertex normals, accumulate face normals in positions
		// (and normalizes them when all triangles were processed or in a shader)
		normals[v0] += normal;
		normals[v1] += normal;
		normals[v2] += normal;
	}

	CreateFlatArraysTri();
}

unsigned int Model::GetSizeOfVertices() const
{
	if (type == ModelType::Tri)
	{
		// Tri models don't have texture coordinates, so no tangents and bitangents either
		// Each vertex contains only its position and normal
		return (unsigned int)(sizeof(float) * positions.size()) * (3 + 3);
	}

	return 0;
}

unsigned int Model::GetNumberOfIndices() const
{
	return (unsigned int)faces.size() * 3u;
}

void Model::CreateFlatArraysTri()
{
	size_t numberOfFaces = faces.size();
	flatIndices = new unsigned int[numberOfFaces * 3];

	for (int i = 0; i < numberOfFaces; i++)
	{
		flatIndices[i * 3 + 0] = (unsigned int)faces[i].v[0].vertex;
		flatIndices[i * 3 + 1] = (unsigned int)faces[i].v[1].vertex;
		flatIndices[i * 3 + 2] = (unsigned int)faces[i].v[2].vertex;
	}

	size_t numberOfVertices = positions.size();
	int stride = 3 + 3;
	flatVertices = new float[numberOfVertices * stride];

	for (int i = 0; i < numberOfVertices; i++)
	{
		flatVertices[i * stride + 0] = positions[i].x;
		flatVertices[i * stride + 1] = positions[i].y;
		flatVertices[i * stride + 2] = positions[i].z;
	
		flatVertices[i * stride + 3] = normals[i].x;
		flatVertices[i * stride + 4] = normals[i].y;
		flatVertices[i * stride + 5] = normals[i].z;
	}
}

Model::~Model()
{
	if (flatVertices) delete flatVertices;
	if (flatIndices) delete flatIndices;
}

} // namespace Hedge
