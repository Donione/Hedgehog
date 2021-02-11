#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
	virtual ~Shader() {};

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void UploadConstant(const std::string& name, float constant) = 0;
	virtual void UploadConstant(const std::string& name, glm::vec2 constant) = 0;
	virtual void UploadConstant(const std::string& name, glm::vec3 constant) = 0;
	virtual void UploadConstant(const std::string& name, glm::vec4 constant) = 0;

	virtual void UploadConstant(const std::string& name, glm::mat3x3 constant) = 0;
	virtual void UploadConstant(const std::string& name, glm::mat4x4 constant) = 0;

	virtual void UploadConstant(const std::string& name, int constant) = 0;

	virtual void UploadConstant(const std::string& name, void* constant, unsigned long long size) = 0;

	static Shader* Create(const std::string& filePath);
	static Shader* Create(const std::string& vertexFilePath, const std::string& pixelFilePath);
};