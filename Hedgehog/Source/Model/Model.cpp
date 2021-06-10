#include <Model/Model.h>

#include <fstream>
#include <limits>

#include <glm/gtx/matrix_decompose.hpp>


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

	size_t dotPos = filename.rfind('.');
	std::string materialFilename = filename.substr(0, dotPos) + ".mtl";
	LoadMtl(materialFilename);

	std::ifstream in(filename);
	char buffer[256];
	std::string line;
	std::stringstream liness;
	std::string text;
	std::string value;
	float x, y, z;
	Face face;
	std::string v[3];
	int currentGroup = -1;
	int currentSmoothingGroup = 0;
	std::string currentMaterial;

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

			if (currentGroup != -1)
			{
				groups[currentGroup].endIndex = (int)faces.size() - 1;
			}
			currentGroup = (int)groups.size();

			// Assume all groups are unique
			VertexGroup newGroup;
			newGroup.name = value;
			newGroup.startIndex = (int)faces.size();
			groups.push_back(newGroup);

			continue;
		}

		if (line.starts_with("usemtl "))
		{
			liness >> text >> currentMaterial;

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

				// Indices in the file are one-indexed
				// Change them to zero-indexed so we can index our vectors
				face.v[i].vertex--;
				face.v[i].texCoord--;
				face.v[i].normal--;

				face.v[i].group = currentGroup;
				face.v[i].smoothingGroup = currentSmoothingGroup;
				face.v[i].material = materials.at(currentMaterial).textureSlot;
			}

			faces.push_back(face);

			continue;
		}
	}

	groups[currentGroup].endIndex = (int)faces.size() - 1;

	CalculateFaceNormals();
	CalculateTangents();
	CreateFlatArraysObj();
	CreateTBNFlatArraysObj();

	CalculateCenters();
}

void Model::LoadDae(const std::string& filename)
{
	type = ModelType::Dae;

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.c_str());
	assert(result);

	auto meshNode = doc.select_node("/COLLADA/library_geometries/geometry/mesh").node();

	{
		auto positionArray = CreateSource<float>(meshNode, "Position");
		for (size_t i = 0; i < positionArray.size(); i += 3)
		{
			positions.emplace_back(positionArray[i], positionArray[i + 1], positionArray[i + 2]);
		}
	}

	{
		auto normalArray = CreateSource<float>(meshNode, "Normal");
		for (size_t i = 0; i < normalArray.size(); i += 3)
		{
			normals.emplace_back(normalArray[i], normalArray[i + 1], normalArray[i + 2]);
		}
	}

	{
		auto texCoords = CreateSource<float>(meshNode, "UV0");
		for (size_t i = 0; i < texCoords.size(); i += 2)
		{
			textureCoordinates.emplace_back(texCoords[i], texCoords[i + 1]);
		}
	}

	{
		auto node = meshNode.child("polylist");
		int numberOfPolygons = node.attribute("count").as_int();
		auto primitiveNode = node.child("p");
		auto primitiveStream = std::stringstream(primitiveNode.first_child().value());

		for (int i = 0; i < numberOfPolygons; i++)
		{
			Face face;

			for (int j = 0; j < 3; j++)
			{
				primitiveStream >> face.v[j].vertex >> face.v[j].normal >> face.v[j].texCoord;

				for (int k = 0; k < 44; k++)
				{
					unsigned int discard;
					primitiveStream >> discard;
				}
			}

			faces.push_back(face);
		}
	}

	std::string controllerID = doc.select_node("/COLLADA/library_controllers/controller").node().attribute("id").value();

	auto skinNode = doc.select_node("/COLLADA/library_controllers/controller/skin").node();

	{
		segmentNames = CreateSource<std::string>(skinNode, controllerID + "-Joints");

		for (auto& segmentName : segmentNames)
		{
			int segmentID = (int)segments.size();
			segmentMap.emplace(segmentName, segmentID);
			segments.emplace_back(Segment(segmentName, segmentID), -1);
		}
	}

	{
		auto offsets = CreateSource<float>(skinNode, controllerID + "-Matrices");
		for (size_t i = 0; i < offsets.size(); i += 16)
		{
			
			segments[i / 16].first.offset = glm::mat4(offsets[i +  0], offsets[i +  4], offsets[i +  8], offsets[i + 12],
													  offsets[i +  1], offsets[i +  5], offsets[i +  9], offsets[i + 13],
													  offsets[i +  2], offsets[i +  6], offsets[i + 10], offsets[i + 14],
													  offsets[i +  3], offsets[i +  7], offsets[i + 11], offsets[i + 15]);
		}
	}

	{
		segmentWeights = CreateSource<float>(skinNode, controllerID + "-Weights");
	}

	{
		auto weightsNode = skinNode.child("vertex_weights");
		int numberOfVertexWeights = weightsNode.attribute("count").as_int();
		auto vcountNode = weightsNode.child("vcount");
		auto vcountStream = std::stringstream(vcountNode.first_child().value());
		auto vnode = weightsNode.child("v");
		auto vStream = std::stringstream(vnode.first_child().value());

		for (int j = 0; j < numberOfVertexWeights; j++)
		{
			unsigned int vcount;
			vcountStream >> vcount;

			SegmentIDs segmentIDindex = { -1, -1, -1, -1 };
			SegmentWeightIndices segmentWeightIndex = { 0, 0, 0, 0 };
			for (unsigned int i = 0; i < vcount; i++)
			{
				if (i < 4)
				{
					vStream >> segmentIDindex.ID[i] >> segmentWeightIndex.i[i];
				}
				else
				{
					int discard1, discard2;
					vStream >> discard1 >> discard2;
				}
			}

			segmentIDs.push_back(segmentIDindex);
			segmentWeightIndices.push_back(segmentWeightIndex);
		}
	}

	{
		auto animationNodes = doc.select_nodes("/COLLADA/library_animations/animation");

		for (auto& animationNode : animationNodes)
		{
			std::string segmentName = std::string(animationNode.node().attribute("name").value());

			auto segmentTimeStamps = CreateSource<float>(animationNode.node(), "Matrix-animation-input");
			auto segmentTransforms = CreateSource<float>(animationNode.node(), "animation-output-transform");

			int segmentID = segmentMap.at(segmentName);
			for (size_t keyFrame = 0; keyFrame < segmentTimeStamps.size(); keyFrame++)
			{
				float timeStamp = segmentTimeStamps[keyFrame];
				glm::mat4 transform(segmentTransforms[keyFrame * 16 +  0], segmentTransforms[keyFrame * 16 +  4], segmentTransforms[keyFrame * 16 +  8], segmentTransforms[keyFrame * 16 + 12],
									segmentTransforms[keyFrame * 16 +  1], segmentTransforms[keyFrame * 16 +  5], segmentTransforms[keyFrame * 16 +  9], segmentTransforms[keyFrame * 16 + 13],
									segmentTransforms[keyFrame * 16 +  2], segmentTransforms[keyFrame * 16 +  6], segmentTransforms[keyFrame * 16 + 10], segmentTransforms[keyFrame * 16 + 14],
									segmentTransforms[keyFrame * 16 +  3], segmentTransforms[keyFrame * 16 +  7], segmentTransforms[keyFrame * 16 + 11], segmentTransforms[keyFrame * 16 + 15]);
				segments[segmentID].first.keyTransforms.emplace_back(timeStamp, transform);

				glm::vec3 scale;
				glm::quat rotation;
				glm::vec3 translation;
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(transform, scale, rotation, translation, skew, perspective);

				segments[segmentID].first.keyPositions.emplace_back(timeStamp, translation);
				segments[segmentID].first.keyRotations.emplace_back(timeStamp, rotation);
				segments[segmentID].first.keyScales.emplace_back(timeStamp, scale);
			}
		}
	}

	{
		auto rootSegment = doc.select_node("/COLLADA/library_visual_scenes/visual_scene").node().first_child();

		auto nodes = rootSegment.select_nodes(".//node");
		for (auto& node : nodes)
		{
			auto childName = node.node().attribute("name").value();
			int childID = segmentMap.at(std::string(childName));

			auto parent = node.parent();
			auto parentName = parent.attribute("name").value();
			int parentID = segmentMap.at(std::string(parentName));

			segments[childID].second = parentID;
		}

		animation = Animation(segments);
	}

	CalculateFaceNormals();
	CalculateTangents();
	CreateFlatArraysObj();
}

unsigned int Model::GetSizeOfVertices() const
{
	return (unsigned int)(sizeof(float) * flatVertices.size());
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

void Model::LoadMtl(const std::string& filename)
{
	std::ifstream in(filename);
	char buffer[256];
	std::string line;
	std::stringstream liness;
	std::string text;
	std::string value;
	std::string currentMaterial;

	size_t dotPos = filename.rfind('\\');
	std::string basePath = filename.substr(0, dotPos + 1);

	while (!in.eof())
	{
		in.getline(buffer, 256);
		line = std::string(buffer);
		liness = std::stringstream(buffer);

		if (line.starts_with("newmtl "))
		{
			liness >> text >> currentMaterial;

			Material material = { currentMaterial, (int)materials.size(), "", "" };
			materials.emplace(currentMaterial, material);
		}

		if (line.starts_with("map_Kd "))
		{
			liness >> text >> value;

			size_t slashPos = value.rfind('/');
			value = value.replace(slashPos, 1, "\\");
			materials.at(currentMaterial).diffuseFilename = basePath + value;
		}

		if (line.starts_with("map_Disp "))
		{
			liness >> text >> value;

			size_t slashPos = value.rfind('/');
			value = value.replace(slashPos, 1, "\\");
			std::string xxx = basePath + value;
			materials.at(currentMaterial).normalFilename = basePath + value;
		}
	}

	CreateTextureDescription();
}

void Model::CreateTextureDescription()
{
	textureDescription.resize(materials.size() * 2);

	for (const auto& [name, material] : materials)
	{
		textureDescription[(size_t)(material.textureSlot) * 2 + 0] = { TextureType::Diffuse, material.diffuseFilename };
		textureDescription[(size_t)(material.textureSlot) * 2 + 1] = { TextureType::Normal, material.normalFilename };
	}
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
	long long stride = (long long)
		  3  // position
		+ 1  // texture slot
		+ 2  // texture coordinates
		+ 3  // normal
		+ 3  // tangent
		+ 3  // bitangent
		+ 4  // segmentIDs
		+ 4; // segmentWeights
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

			flatVertices[index * stride + 3] = (float)face.v[i].material == -1 ? 0 : (float)face.v[i].material;
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

			flatVertices[index * stride + 15] = type == ModelType::Dae ? (float)segmentIDs[face.v[i].vertex].ID[0] :  0.0f;
			flatVertices[index * stride + 16] = type == ModelType::Dae ? (float)segmentIDs[face.v[i].vertex].ID[1] : -1.0f;
			flatVertices[index * stride + 17] = type == ModelType::Dae ? (float)segmentIDs[face.v[i].vertex].ID[2] : -1.0f;
			flatVertices[index * stride + 18] = type == ModelType::Dae ? (float)segmentIDs[face.v[i].vertex].ID[3] : -1.0f;

			flatVertices[index * stride + 19] = type == ModelType::Dae ? segmentWeights[segmentWeightIndices[face.v[i].vertex].i[0]] : 1.0f;
			flatVertices[index * stride + 20] = type == ModelType::Dae ? segmentWeights[segmentWeightIndices[face.v[i].vertex].i[1]] : 0.0f;
			flatVertices[index * stride + 21] = type == ModelType::Dae ? segmentWeights[segmentWeightIndices[face.v[i].vertex].i[2]] : 0.0f;
			flatVertices[index * stride + 22] = type == ModelType::Dae ? segmentWeights[segmentWeightIndices[face.v[i].vertex].i[3]] : 0.0f;
		}
	}
}

void Model::CreateTBNFlatArraysObj()
{
	assert(!indices.empty());

	size_t numberOfVertices = indices.size();
	flatTBNIndices.resize(numberOfVertices * 3 * 2);
	long long int stride = (long long)4 + 4 + 2 + 1;
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

		flatTBNVertices[index * stride + 10] = -1.0f;


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

		flatTBNVertices[normalIndex * stride + 10] = -1.0f;


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

		flatTBNVertices[tangentIndex * stride + 10] = -1.0f;


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

		flatTBNVertices[bitangentIndex * stride + 10] = -1.0f;
	}
}

void Model::CalculateCenters()
{
	for (auto& group : groups)
	{
		glm::vec3 min{ std::numeric_limits<float>::infinity() };
		glm::vec3 max{ -std::numeric_limits<float>::infinity() };

		for (unsigned int index = group.startIndex; index <= group.endIndex; index++)
		{
			for (int i = 0; i < 3; i++)
			{
				glm::vec3 vertex = positions[faces[index].v[i].vertex];

				max = glm::max(vertex, max);
				min = glm::min(vertex, min);
			}
		}

		glm::vec3 center = min + ((max - min) / 2.0f);

		// TODO This is just a temporary wokaround specifically for the sponza model
		// Only the following three materials have alpha channels in their textures,
		// so we want only those three  to be sorted by distance from the camera,
		// the rest should be rendered before them
		if (faces[group.startIndex].v[0].material == 3
			|| faces[group.startIndex].v[0].material == 7
			|| faces[group.startIndex].v[0].material == 20)
		{
			group.center = center;
		}
		else
		{
			group.center = center + 100000.0f;
		}
	}
}

Model::~Model()
{
}

} // namespace Hedge
