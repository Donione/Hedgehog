#pragma once

#include <Renderer/Shader.h>

#include <glad/glad.h>
#include <vector>


namespace Hedge
{

class OpenGLShader : public Shader
{
public:
	OpenGLShader(const std::string& vertexFilePath, const std::string& pixelFilePath);
	virtual ~OpenGLShader() override;

	virtual void Bind() override;
	virtual void Unbind() override;

	virtual void SetupConstantBuffers(ConstantBufferDescription constBufferDesc) override { /* do nothing */ }

	virtual void UploadConstant(const std::string& name, float constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec2 constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec3 constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec4 constant) override;

	virtual void UploadConstant(const std::string& name, glm::mat3x3 constant) override;
	virtual void UploadConstant(const std::string& name, glm::mat4x4 constant) override;

	virtual void UploadConstant(const std::string& name, int constant) override;

	virtual void UploadConstant(const std::string& name, void* constant, unsigned long long size) override;

private:
	std::string ReadFile(const std::string& filePath);

	std::vector<GLchar> getShaderInfoLog(GLint id);

private:
	unsigned int shaderID = 0;
};

} // namespace Hedge