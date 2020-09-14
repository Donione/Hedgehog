#pragma once

#include <string>

namespace Messaging
{
	// TODECIDE: Can a single message belong to multiple message classes?
	enum class MessageCategory
	{
		Undefined = 0
	};

	enum class MessageType
	{
		Undefined = 0
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

	private:
		MessageCategory category = MessageCategory::Undefined;
		MessageType type = MessageType::Undefined;
	};
} // namespace Messaging