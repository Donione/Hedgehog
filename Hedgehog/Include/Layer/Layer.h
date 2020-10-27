#pragma once

#include <string>

#include <Message/Message.h>


class Layer
{
public:
	Layer(const std::string& name = "Hedgehog Layer") : name(name) {}
	//Virtual destructors are useful when you might potentially delete an instance of a derived class
	// through a pointer to base class
	// https://stackoverflow.com/questions/461203/when-to-use-virtual-destructors
	virtual ~Layer() {}

	virtual void OnPush() {}
	virtual void OnPop() {}
	virtual void OnUpdate() {}
	virtual void OnMessage(const Message& message) {}

	const std::string& GetName() const { return name; }

protected:
	std::string name;

	bool enabled = true;
};