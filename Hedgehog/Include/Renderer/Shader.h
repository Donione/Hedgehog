#pragma once

#include <Component/Light.h>

#include <string>
#include <glm/glm.hpp>
#include <vector>


namespace Hedge
{

enum class ConstantBufferUsage
{
	Scene,
	Light,
	Object,
	Other,
};

// TODO setup tiered approch for constatnt data
// something like:
//    something that need to be accessed very frequently/very fast -> root constant
//    constant per draw call (like a transform matrix) -> root descriptor
//    constant per frame (maybe a projection view matrix) -> descriptor table
struct ConstantBufferDescriptionElement
{
	ConstantBufferDescriptionElement() = default;
	ConstantBufferDescriptionElement(const std::string& name,
									 unsigned long long size,
									 ConstantBufferUsage usage = ConstantBufferUsage::Other,
									 unsigned int count = 1)
		: name(name), size(size), usage(usage), count(count) {}

	std::string name;
	unsigned long long size;
	ConstantBufferUsage usage;
	unsigned int count;
};

class ConstantBufferDescription
{
public:
	ConstantBufferDescription() = default;
	ConstantBufferDescription(const std::initializer_list<ConstantBufferDescriptionElement>& elements)
		: description(elements) {}

	size_t Size() const { return description.size(); }
	const std::vector<ConstantBufferDescriptionElement>& Get() const { return description; }

	std::vector<ConstantBufferDescriptionElement>::iterator begin() { return description.begin(); }
	std::vector<ConstantBufferDescriptionElement>::iterator end() { return description.end(); }
	std::vector<ConstantBufferDescriptionElement>::const_iterator begin() const { return description.begin(); }
	std::vector<ConstantBufferDescriptionElement>::const_iterator end() const { return description.end(); }

private:
	std::vector<ConstantBufferDescriptionElement> description;
};


class Shader
{
public:
	virtual ~Shader() {};

	virtual void Bind() = 0;
	virtual void Unbind() = 0;

	virtual void SetupConstantBuffers(ConstantBufferDescription constBufferDesc) { /* Do Nothing by default */ }

	virtual void UploadConstant(const std::string& name, float constant) = 0;
	virtual void UploadConstant(const std::string& name, glm::vec2 constant) = 0;
	virtual void UploadConstant(const std::string& name, glm::vec3 constant) = 0;
	virtual void UploadConstant(const std::string& name, glm::vec4 constant) = 0;

	virtual void UploadConstant(const std::string& name, glm::mat3x3 constant) = 0;
	virtual void UploadConstant(const std::string& name, glm::mat4x4 constant) = 0;

	virtual void UploadConstant(const std::string& name, const std::vector<glm::mat4>& constant) = 0;

	virtual void UploadConstant(const std::string& name, int constant) = 0;

	virtual void UploadConstant(const std::string& name, const DirectionalLight& constant) = 0;
	virtual void UploadConstant(const std::string& name, const PointLight& constant) = 0;
	virtual void UploadConstant(const std::string& name, const SpotLight& constant) = 0;

	virtual void UploadConstant(const std::string& name, const DirectionalLight* constant, int count = 1) = 0;
	virtual void UploadConstant(const std::string& name, const PointLight* constant, int count = 1) = 0;
	virtual void UploadConstant(const std::string& name, const SpotLight* constant, int count = 1) = 0;

	virtual void UploadConstant(const std::string& name, const void* constant, unsigned long long size) = 0;

	virtual const size_t GetConstBufferCount() const { return 0; }

	static Shader* Create(const std::string& filePath);
	static Shader* Create(const std::string& vertexFilePath,
						  const std::string& pixelFilePath,
						  const std::string& geometryFilePath = "");
};

} // namespace Hedge