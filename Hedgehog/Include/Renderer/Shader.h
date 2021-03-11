#pragma once

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
									 ConstantBufferUsage usage = ConstantBufferUsage::Other)
		: name(name), size(size), usage(usage) {}

	std::string name;
	unsigned long long size;
	ConstantBufferUsage usage;
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

	virtual void SetupConstantBuffers(ConstantBufferDescription constBufferDesc) = 0;

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

} // namespace Hedge