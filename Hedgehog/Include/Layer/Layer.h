#pragma once

#include <string>
#include <chrono>

#include <Message/Message.h>


class Layer
{
protected:
	Layer(const std::string& name = "Hedgehog Layer", bool enabled = true) : name(name), enabled(enabled) {}

public:
	//Virtual destructors are useful when you might potentially delete an instance of a derived class
	// through a pointer to base class
	// https://stackoverflow.com/questions/461203/when-to-use-virtual-destructors
	virtual ~Layer() {}

	virtual void OnPush() {}
	virtual void OnPop() {}
	virtual void OnUpdate(const std::chrono::duration<double, std::milli>& duration) {}
	virtual void OnGuiUpdate() {}
	virtual void OnMessage(const Message& message) {}

	const std::string& GetName() const { return name; }
	void Enable() { enabled = true; }
	void Disable() { enabled = false; }
	bool IsEnabled() { return enabled; }

protected:
	const std::string name;
	bool enabled;
};