#pragma once

#include <string>

namespace Morris
{
	class IMorrisLogger
	{
	public:
		virtual ~IMorrisLogger() {};
		virtual void OnLog(const std::string& message) = 0;
	};
}