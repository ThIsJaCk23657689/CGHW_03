#ifndef LOGGING_H
#define LOGGING_H

#include <ctime>
#include <string>
#include <iostream>

namespace logging {
	// This is for the argument "type" which in the loggingMessage().
	enum LogType {
		DEBUG,
		INFO,
		WARNING,
		ERROR,
	};

	// Generate timestamp for logging
	std::string getTimestamp(void) {
		time_t timer = std::time(0);
		std::tm bt{};
		localtime_s(&bt, &timer);

		char buffer[64];
		return { buffer, std::strftime(buffer, sizeof(buffer), "%F %T ", &bt) };
	}

	// Logging out the message.
	void loggingMessage(int type, std::string message) {
		switch (type) {
		case DEBUG:
			std::cout << getTimestamp() << "[DEBUG] " << message << std::endl;
			break;
		case INFO:
			std::cout << getTimestamp() << "[INFO] " << message << std::endl;
			break;
		case WARNING:
			std::cout << getTimestamp() << "[WARNING] " << message << std::endl;
			break;
		case ERROR:
			std::cerr << getTimestamp() << "[ERROR] " << message << std::endl;
			break;
		default:
			std::cerr << getTimestamp() << "[ERROR] " << "Undefined type in loggingMessage()." << std::endl;
			break;
		}
	}

	void showInitInfo(const GLubyte* renderer, const GLubyte* version) {
		std::cout << getTimestamp() << "[INFO] GPU: " << renderer << std::endl;
		std::cout << getTimestamp() << "[INFO] OpenGL Version: " << version << std::endl;
	}
}

#endif // !LOGGING_H