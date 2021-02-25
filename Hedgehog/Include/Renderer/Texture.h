#pragma once

#include <string>


namespace Hedge
{

class Texture
{
public:
	virtual ~Texture() = default;

	virtual void Bind(unsigned int slot = 0) const = 0;

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;
};


class Texture2D : public Texture
{
public:
	static Texture2D* Create(const std::string& filename);
};

} // namespace Hedge
