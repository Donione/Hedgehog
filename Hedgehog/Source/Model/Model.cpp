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

		Face f;
		f.v[0].vertex = f.v[0].normal = (int)v0;
		f.v[1].vertex = f.v[1].normal = (int)v1;
		f.v[2].vertex = f.v[2].normal = (int)v2;
		faces.push_back(f);

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
	std::string value;
	float x, y, z;
	Face face;
	std::string v[3];
	int currentGroup = 0;
	int currentSmoothingGroup = 0;

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
			liness >> text >> value;

			if (!groups.contains(value))
			{
				groups.emplace(value, (int)groups.size());
			}
			currentGroup = groups.at(value);

			continue;
		}

		if (line.starts_with("usemtl "))
		{
			continue;
		}

		if (line.starts_with("s "))
		{
			liness >> text >> value;

			if (value == "off")
			{
				currentSmoothingGroup = 0;
			}
			else
			{
				currentSmoothingGroup = std::stoi(value);
			}

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

				face.v[i].group = currentGroup;
				face.v[i].smoothingGroup = currentSmoothingGroup;
			}

			faces.push_back(face);

			continue;
		}
	}

	CalculateFaceNormals();
	CalculateTangents();
	CreateFlatArraysObj();
	CreateTBNFlatArraysObj();
}

unsigned int Model::GetSizeOfVertices() const
{
	if (type == ModelType::Tri)
	{
		// Tri models don't have texture coordinates, so no tangents and bitangents either
		// Each vertex contains only its position and normal
		return (unsigned int)(sizeof(float) * flatVertices.size());
	}
	else if (type == ModelType::Obj)
	{
		// Each vertex contains its position, texture coordinates, normal, tangent and bitangent
		return (unsigned int)(sizeof(float) * flatVertices.size());
	}

	return 0;
}

unsigned int Model::GetNumberOfIndices() const
{
	return (unsigned int)flatIndices.size();
}

unsigned int Model::GetSizeOfTBNVertices() const
{
	return (unsigned int)(sizeof(float) * flatTBNVertices.size());
}

unsigned int Model::GetNumberOfTBNIndices() const
{
	return (unsigned int)flatTBNIndices.size();
}

void Model::CalculateFaceNormals()
{
	auto comp = [](const glm::vec3& a, const glm::vec3& b)
	{
		if (a.x != b.x)
		{
			return a.x < b.x;
		}
		else
		{
			if (a.y != b.y)
			{
				return a.y < b.y;
			}
			else
			{
				return a.z < b.z;
			}
		}
	};

	std::map<glm::vec3, int, decltype(comp)> faceNormalMap(comp);

	int faceNormalIndex = 0;
	for (auto& face : faces)
	{
		glm::vec3 faceNormal = glm::normalize(glm::cross(positions[face.v[1].vertex] - positions[face.v[0].vertex],
														 positions[face.v[2].vertex] - positions[face.v[0].vertex]));

		// Check if this face normal already exists
		if (faceNormalMap.contains(faceNormal))
		{
			faceNormalIndex = faceNormalMap.at(faceNormal);
		}
		else
		{
			faceNormalIndex = (int)faceNormals.size();
			faceNormalMap.emplace(faceNormal, faceNormalIndex);
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
	assert(!faces.empty());

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
		// When two of the texture coordinates are the same (the face is mapped onto the texture as a line)
		// the f is infinity and breaks the TBN calculations
		// Just put zero here so T and B are also zero and hope for the best for now
		// TODO handle this gracefully
		if (isinf(f))
		{
			f = 0.0f;
		}

		// We still have an issue when faces meet in a vertex with opposite B and T
		// When accumulated together they mangle or completely cancel each other
		// causing jarring lighting differences between faces or no lighting at all.
		// We need to either detect these cases and not smooth the tangents or change the model to include hard edges
		// Until we resolve that issue, the tangent smoothing is disabled
		// This works around the edge cases (hehe, "Edge cases", get it?) with the cost of visible edges between larger faces
		// and increase in the number of vertices that are uploaded into the GPU

		for (int i = 0; i < 3; i++)
		{
			int index;
			//if (face.v[i].smoothingGroup == 0)
			// Disable tangent smoothing for now
			if (true)
			{
				index = (int)tangents.size();
				tangents.emplace_back(0.0f, 0.0f, 0.0f);
				bitangents.emplace_back(0.0f, 0.0f, 0.0f);
			}
			else
			{
				if (groupIndices.contains(face.v[i]))
				{
					index = groupIndices.at(face.v[i]);
				}
				else
				{
					index = (int)tangents.size();
					tangents.emplace_back(0.0f, 0.0f, 0.0f);
					bitangents.emplace_back(0.0f, 0.0f, 0.0f);
					groupIndices.emplace(face.v[i], index);
				}
			}

			face.v[i].tangent = index;

			glm::vec3 T = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
			glm::vec3 B = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
			glm::vec3 N = normals[face.v[i].normal];

			// When symmetric models are used, UVs are oriented in the wrong way, and the T has the wrong orientation.
			// TBN must form a right-handed coordinate system, i.e. cross(N,T) must have the same orientation as B.
			if (glm::dot(glm::cross(N, T), B) < 0.0f)
			{
				T = T * -1.0f;
			}

			tangents[index] += T;
			bitangents[index] += B;
		}
	}

	// When we're accumulating the tangents for shared vertices, the TBN system might become not orthogonal
	// We can re-orthogonalize T with respect to N and retrieve the B
	// At the moment, we're doing this in the vertex shader
	//{
	//	T = glm::normalize(T - N * glm::dot(N, T));
	//	B = cross(N, T);
	//}
}

void Model::CreateFlatArraysTri()
{
	size_t numberOfFaces = faces.size();
	flatIndices.resize(numberOfFaces * 3);

	for (size_t i = 0; i < numberOfFaces; i++)
	{
		flatIndices[i * 3 + 0] = (unsigned int)faces[i].v[0].vertex;
		flatIndices[i * 3 + 1] = (unsigned int)faces[i].v[1].vertex;
		flatIndices[i * 3 + 2] = (unsigned int)faces[i].v[2].vertex;
	}

	size_t numberOfVertices = positions.size();
	int stride = 3 + 3;
	flatVertices.resize(numberOfVertices * stride);

	for (size_t i = 0; i < numberOfVertices; i++)
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
	flatIndices.resize(numberOfFaces * 3);

	size_t numberOfVertices = indices.size();
	long long stride = (long long)3 + 1 + 2 + 3 + 3 + 3;
	flatVertices.resize(numberOfVertices * stride);

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

			flatVertices[index * stride + 3] = 0.0f;
			flatVertices[index * stride + 4] = textureCoordinates[face.v[i].texCoord].x;
			flatVertices[index * stride + 5] = textureCoordinates[face.v[i].texCoord].y;

			flatVertices[index * stride + 6] = normals[face.v[i].normal].x;
			flatVertices[index * stride + 7] = normals[face.v[i].normal].y;
			flatVertices[index * stride + 8] = normals[face.v[i].normal].z;

			flatVertices[index * stride +  9] = tangents[face.v[i].tangent].x;
			flatVertices[index * stride + 10] = tangents[face.v[i].tangent].y;
			flatVertices[index * stride + 11] = tangents[face.v[i].tangent].z;

			flatVertices[index * stride + 12] = bitangents[face.v[i].tangent].x;
			flatVertices[index * stride + 13] = bitangents[face.v[i].tangent].y;
			flatVertices[index * stride + 14] = bitangents[face.v[i].tangent].z;
		}
	}
}

void Model::CreateTBNFlatArraysObj()
{
	assert(!indices.empty());

	size_t numberOfVertices = indices.size();
	flatTBNIndices.resize(numberOfVertices * 3 * 2);
	long long int stride = (long long)4 + 4 + 2;
	flatTBNVertices.resize(numberOfVertices * 4 * stride);

	int flatIndex = 0;
	for (auto& [vertex, index] : indices)
	{
		flatTBNIndices[flatIndex++] = index;

		flatTBNVertices[index * stride + 0] = positions[vertex.vertex].x;
		flatTBNVertices[index * stride + 1] = positions[vertex.vertex].y;
		flatTBNVertices[index * stride + 2] = positions[vertex.vertex].z;
		flatTBNVertices[index * stride + 3] = 1.0f;

		flatTBNVertices[index * stride + 4] = 1.0f;
		flatTBNVertices[index * stride + 5] = 1.0f;
		flatTBNVertices[index * stride + 6] = 1.0f;
		flatTBNVertices[index * stride + 7] = 1.0f;

		flatTBNVertices[index * stride + 8] = 0.0f;
		flatTBNVertices[index * stride + 9] = 0.0f;


		// Normals
		unsigned int normalIndex = (int)numberOfVertices + index;
		flatTBNIndices[flatIndex++] = normalIndex;

		flatTBNVertices[normalIndex * stride + 0] = positions[vertex.vertex].x + normals[vertex.normal].x * 3;
		flatTBNVertices[normalIndex * stride + 1] = positions[vertex.vertex].y + normals[vertex.normal].y * 3;
		flatTBNVertices[normalIndex * stride + 2] = positions[vertex.vertex].z + normals[vertex.normal].z * 3;
		flatTBNVertices[normalIndex * stride + 3] = 1.0f;

		flatTBNVertices[normalIndex * stride + 4] = 0.0f;
		flatTBNVertices[normalIndex * stride + 5] = 1.0f;
		flatTBNVertices[normalIndex * stride + 6] = 0.0f;
		flatTBNVertices[normalIndex * stride + 7] = 1.0f;

		flatTBNVertices[normalIndex * stride + 8] = 0.0f;
		flatTBNVertices[normalIndex * stride + 9] = 0.0f;


		// Tangents
		unsigned int tangentIndex = (int)numberOfVertices * 2 + index;
		flatTBNIndices[flatIndex++] = index;
		flatTBNIndices[flatIndex++] = tangentIndex;

		glm::vec3 tangent = glm::normalize(tangents[vertex.tangent]);

		flatTBNVertices[tangentIndex * stride + 0] = positions[vertex.vertex].x + tangent.x * 3;
		flatTBNVertices[tangentIndex * stride + 1] = positions[vertex.vertex].y + tangent.y * 3;
		flatTBNVertices[tangentIndex * stride + 2] = positions[vertex.vertex].z + tangent.z * 3;
		flatTBNVertices[tangentIndex * stride + 3] = 1.0f;

		flatTBNVertices[tangentIndex * stride + 4] = 1.0f;
		flatTBNVertices[tangentIndex * stride + 5] = 0.0f;
		flatTBNVertices[tangentIndex * stride + 6] = 0.0f;
		flatTBNVertices[tangentIndex * stride + 7] = 1.0f;

		flatTBNVertices[tangentIndex * stride + 8] = 0.0f;
		flatTBNVertices[tangentIndex * stride + 9] = 0.0f;


		// Bitangents
		unsigned int bitangentIndex = (int)numberOfVertices * 3 + index;
		flatTBNIndices[flatIndex++] = index;
		flatTBNIndices[flatIndex++] = bitangentIndex;

		glm::vec3 bitangent = glm::normalize(bitangents[vertex.tangent]);

		flatTBNVertices[bitangentIndex * stride + 0] = positions[vertex.vertex].x + bitangent.x * 3;
		flatTBNVertices[bitangentIndex * stride + 1] = positions[vertex.vertex].y + bitangent.y * 3;
		flatTBNVertices[bitangentIndex * stride + 2] = positions[vertex.vertex].z + bitangent.z * 3;
		flatTBNVertices[bitangentIndex * stride + 3] = 1.0f;

		flatTBNVertices[bitangentIndex * stride + 4] = 0.0f;
		flatTBNVertices[bitangentIndex * stride + 5] = 0.0f;
		flatTBNVertices[bitangentIndex * stride + 6] = 1.0f;
		flatTBNVertices[bitangentIndex * stride + 7] = 1.0f;

		flatTBNVertices[bitangentIndex * stride + 8] = 0.0f;
		flatTBNVertices[bitangentIndex * stride + 9] = 0.0f;
	}
}

Model::~Model()
{
}

} // namespace Hedge
