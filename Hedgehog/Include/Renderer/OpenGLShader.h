#pragma once

#include <Renderer/Shader.h>

#include <glad/glad.h>
#include <vector>


class OpenGLShader : public Shader
{
public:
	OpenGLShader(const std::string& vertexFilePath, const std::string& pixelFilePath);
	virtual ~OpenGLShader() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

	virtual void UploadUniform(const std::string& name, glm::mat4x4 uniform) const override;

private:
	std::string ReadFile(const std::string& filePath);

	std::vector<GLchar> getShaderInfoLog(GLint id);

private:
	unsigned int shaderID = 0;
};