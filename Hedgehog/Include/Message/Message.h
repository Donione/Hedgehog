#pragma once

#include <string>
#include <sstream>
#include <iomanip>


// TODECIDE: Can a single message belong to multiple message classes?
enum class MessageCategory
{
	Undefined = 0,
	Key,
	Mouse,
};

enum class MessageType
{
	Undefined = 0,
	KeyPressed, KeyReleased,
	MouseMoved, MouseScrolled,
};

// Base Message Class
class Message
{
public:
	MessageCategory GetMessageCategory() const;
	MessageType GetMessageType() const;

	virtual std::string ToString() const = 0;

	bool IsInClass(MessageCategory messageClass) const;

protected:
	bool wasHandled = false;
	MessageCategory category = MessageCategory::Undefined;
	MessageType type = MessageType::Undefined;
};