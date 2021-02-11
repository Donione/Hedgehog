#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
	virtual ~Shader() {};

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void UploadUniform(const std::string& name, float uniform) = 0;
	virtual void UploadUniform(const std::string& name, glm::vec2 uniform) = 0;
	virtual void UploadUniform(const std::string& name, glm::vec3 uniform) = 0;
	virtual void UploadUniform(const std::string& name, glm::vec4 uniform) = 0;

	virtual void UploadUniform(const std::string& name, glm::mat3x3 uniform) = 0;
	virtual void UploadUniform(const std::string& name, glm::mat4x4 uniform) = 0;

	virtual void UploadUniform(const std::string& name, int uniform) = 0;

	virtual void UploadUniform(const std::string& name, void* uniform, unsigned long long size) = 0;

	static Shader* Create(const std::string& vertexFilePath, const std::string& pixelFilePath);
};