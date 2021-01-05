#pragma once

#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
	virtual ~Shader() {};

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual void UploadUniform(const std::string& name, glm::mat4x4 uniform) const = 0;

	static Shader* Create(const std::string& vertexFilePath, const std::string& pixelFilePath);
};