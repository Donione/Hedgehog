#pragma once

#include <string>
#include <vector>


namespace Hedge
{

// TODO matrices
enum class ShaderDataType
{
	None = 0,
	Float,
	Float2,
	Float3,
	Float4,
	Int,
	Int2,
	Int3,
	Int4,
	Bool
};

enum class PrimitiveTopology
{
	None = 0,
	Point,
	Line,
	Triangle,
};

struct ShaderDataTypeAttributes
{
	unsigned int size;
	unsigned int count;
};

// TODECIDE we kinda have redundant information here, size is just count * sizeof(basetype)
// Also it gets fuzzy when we add matrices, especially if we want to support non square matrices
static const ShaderDataTypeAttributes shaderDataTypeAttributes[] =
{
	{ 0, 0 }, // None
	{ 4, 1 }, // Float
	{ 4 * 2, 2 }, // Float2
	{ 4 * 3, 3 }, // Float3
	{ 4 * 4, 4 }, // Float4
	{ 4, 1 }, // Int
	{ 4 * 2, 2 }, // Int2
	{ 4 * 3, 3 }, // Int3
	{ 4 * 4, 4 }, // Int4
	{ 1, 1 }, // Bool
};

static unsigned int GetShaderDataTypeSize(ShaderDataType type) { return shaderDataTypeAttributes[(int)type].size; }
static unsigned int GetShaderDataTypeCount(ShaderDataType type) { return shaderDataTypeAttributes[(int)type].count; }

struct BufferElement
{
	BufferElement() = default;
	BufferElement(ShaderDataType type,
				  const std::string& name,
				  int instanceDataStep = -1,
				  bool normalized = false)
		:
		type(type),
		name(name),
		instanceDataStep(instanceDataStep),
		normalized(normalized),
		inputSlot(0), // used by DirectX only
		size(GetShaderDataTypeSize(type)),
		offset(0) {}

	ShaderDataType type;
	std::string name;
	unsigned int size;
	int instanceDataStep;
	unsigned int inputSlot;
	bool normalized;
	// For OpenGL, the offset is later used via casting it to void*
	// having 'unsigned int offset' is a problem on 64-bit since void* is larger
	size_t offset;
};

class BufferLayout
{
public:
	BufferLayout() = default;
	BufferLayout(const std::initializer_list<BufferElement>& elements)
		: elements(elements)
	{
		CalculateOffsetsAndStride();
	}

	const std::vector<BufferElement>& getElements() const { return elements; }
	unsigned int GetStride() const { return stride; }

	std::vector<BufferElement>::iterator begin() { return elements.begin(); }
	std::vector<BufferElement>::iterator end() { return elements.end(); }
	std::vector<BufferElement>::const_iterator begin() const { return elements.begin(); }
	std::vector<BufferElement>::const_iterator end() const { return elements.end(); }

	BufferLayout operator + (const BufferLayout& other) const;
	BufferLayout& operator += (const BufferLayout& other);

private:
	void CalculateOffsetsAndStride();


private:
	std::vector<BufferElement> elements;
	unsigned int stride = 0;
};

class VertexBuffer
{
public:
	static VertexBuffer* Create(const BufferLayout& layout,
								const float* vertices,
								unsigned int size);
	virtual ~VertexBuffer() {}

	virtual void Bind(unsigned int slot = 0) const = 0;
	virtual void Unbind() const = 0;

	virtual const BufferLayout& GetLayout() const = 0;

	virtual void SetData(const float* vertices, unsigned int size) = 0;
};

class IndexBuffer
{
public:
	virtual ~IndexBuffer() {}

	virtual void Bind() const = 0;
	virtual void Unbind() const = 0;

	virtual unsigned int GetCount() const = 0;

	static IndexBuffer* Create(const unsigned int* indices, unsigned int count);
};

} // namespace Hedge
