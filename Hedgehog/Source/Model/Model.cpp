#include <Model/Model.h>

#include <fstream>
#include <sstream>
#include <algorithm>


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

	CalculateFaceNormals();
	CalculateTangents();
	CreateFlatArraysObj();
}

unsigned int Model::GetSizeOfVertices() const
{
	if (type == ModelType::Tri)
	{
		// Tri models don't have texture coordinates, so no tangents and bitangents either
		// Each vertex contains only its position and normal
		return (unsigned int)(sizeof(float) * positions.size()) * (3 + 3);
	}
	else if (type == ModelType::Obj)
	{
		// Each vertex contains its position, texture coordinates, normal, tangent and bitangent
		return (unsigned int)(sizeof(float) * indices.size()) * (3 + 2 + 3 + 3 + 3);
	}

	return 0;
}

unsigned int Model::GetNumberOfIndices() const
{
	return (unsigned int)faces.size() * 3u;
}

void Model::CalculateFaceNormals()
{
	int faceNormalIndex = 0;
	for (auto& face : faces)
	{
		glm::vec3 faceNormal = glm::normalize(glm::cross(positions[face.v[1].vertex] - positions[face.v[0].vertex],
														 positions[face.v[2].vertex] - positions[face.v[0].vertex]));

		// Check if this face normal already exists
		auto it = std::find(faceNormals.begin(), faceNormals.end(), faceNormal);
		if (it != faceNormals.end())
		{
			faceNormalIndex = (int)std::distance(faceNormals.begin(), it);
		}
		else
		{
			faceNormalIndex = (int)faceNormals.size();
			faceNormals.push_back(faceNormal);
		}

		face.v[0].faceNormal = faceNormalIndex;
		face.v[1].faceNormal = faceNormalIndex;
		face.v[2].faceNormal = faceNormalIndex;
	}
}

void Model::MapIndices()
{
	unsigned int index = 0;
	for (auto& face : faces)
	{
		for (int i = 0; i < 3; i++)
		{
			auto vertex = indices.insert({ face.v[i], index });
			if (vertex.second) index++;
		}
	}
}

void Model::CalculateTangents()
{
	assert(!faceNormals.empty());

	tangents.resize(faceNormals.size());
	bitangets.resize(faceNormals.size());

	for (auto& face : faces)
	{
		glm::vec3 pos1, pos2, pos3;
		glm::vec2 uv1, uv2, uv3;

		pos1 = positions[face.v[0].vertex];
		pos2 = positions[face.v[1].vertex];
		pos3 = positions[face.v[2].vertex];

		uv1 = textureCoordinates[face.v[0].texCoord];
		uv2 = textureCoordinates[face.v[1].texCoord];
		uv3 = textureCoordinates[face.v[2].texCoord];

		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		for (int i = 0; i < 3; i++)
		{
			unsigned int index = face.v[i].faceNormal;

			tangents[index].x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			tangents[index].y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			tangents[index].z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			bitangets[index].x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			bitangets[index].y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			bitangets[index].z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		}
	}
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

void Model::CreateFlatArraysObj()
{
	MapIndices();

	size_t numberOfFaces = faces.size();
	flatIndices = new unsigned int[numberOfFaces * 3];

	size_t numberOfVertices = indices.size();
	int stride = 3 + 2 + 3 + 3 + 3;
	flatVertices = new float[numberOfVertices * stride];

	unsigned int flatIndex = 0;
	for (auto& face : faces)
	{
		for (int i = 0; i < 3; i++)
		{
			auto index = indices.at(face.v[i]);

			flatIndices[flatIndex++] = index;
			
			flatVertices[index * stride + 0] = positions[face.v[i].vertex].x;
			flatVertices[index * stride + 1] = positions[face.v[i].vertex].y;
			flatVertices[index * stride + 2] = positions[face.v[i].vertex].z;

			flatVertices[index * stride + 3] = textureCoordinates[face.v[i].texCoord].x;
			flatVertices[index * stride + 4] = textureCoordinates[face.v[i].texCoord].y;

			flatVertices[index * stride + 5] = faceNormals[face.v[i].faceNormal].x;
			flatVertices[index * stride + 6] = faceNormals[face.v[i].faceNormal].y;
			flatVertices[index * stride + 7] = faceNormals[face.v[i].faceNormal].z;

			flatVertices[index * stride + 8] = tangents[face.v[i].faceNormal].x;
			flatVertices[index * stride + 9] = tangents[face.v[i].faceNormal].y;
			flatVertices[index * stride + 10] = tangents[face.v[i].faceNormal].z;

			flatVertices[index * stride + 11] = bitangets[face.v[i].faceNormal].x;
			flatVertices[index * stride + 12] = bitangets[face.v[i].faceNormal].y;
			flatVertices[index * stride + 13] = bitangets[face.v[i].faceNormal].z;
		}
	}
}

Model::~Model()
{
	if (flatVertices) delete flatVertices;
	if (flatIndices) delete flatIndices;
}

} // namespace Hedge
