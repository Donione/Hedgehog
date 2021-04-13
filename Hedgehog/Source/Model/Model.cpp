#include <Model/Model.h>

#include <fstream>
#include <sstream>


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

void Model::LoadObj(const std::string& filename)
{
	type = ModelType::Obj;

	std::ifstream in(filename);
	char buffer[256];
	std::string line;
	std::stringstream liness;
	std::string text;
	float x, y, z;
	Face face;
	std::string v[3];

	while (!in.eof())
	{
		in.getline(buffer, 256);
		line = std::string(buffer);
		liness = std::stringstream(buffer);

		if (line.starts_with('#'))
		{
			continue;
		}

		if (line.starts_with("mtllib"))
		{
			continue;
		}

		if (line.starts_with("o "))
		{
			continue;
		}

		if (line.starts_with("v "))
		{
			liness >> text >> x >> y >> z;

			// Just for testing, put the sponza_236_column_b in the center-ish
			x += 433.0f;
			y -= 680.0f;
			z -= 230.0f;

			positions.emplace_back(x, y, z);

			continue;
		}

		if (line.starts_with("vt "))
		{
			liness >> text >> x >> y;
			textureCoordinates.emplace_back(x, y);

			continue;
		}

		if (line.starts_with("vn "))
		{
			liness >> text >> x >> y >> z;
			normals.emplace_back(x, y, z);

			continue;
		}

		if (line.starts_with("g "))
		{
			continue;
		}

		if (line.starts_with("usemtl "))
		{
			continue;
		}

		if (line.starts_with("s "))
		{
			continue;
		}

		if (line.starts_with("f "))
		{
			liness >> text >> v[0] >> v[1] >> v[2];

			for (int i = 0; i < 3; i++)
			{
				std::stringstream vss(v[i]);
				vss.getline(buffer, 256, '/');
				std::stringstream(buffer) >> face.v[i].vertex;
				vss.getline(buffer, 256, '/');
				std::stringstream(buffer) >> face.v[i].texCoord;
				vss.getline(buffer, 256, '/');
				std::stringstream(buffer) >> face.v[i].normal;

				face.v[i].vertex--;
				face.v[i].texCoord--;
				face.v[i].normal--;
			}

			faces.push_back(face);

			continue;
		}
	}
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
