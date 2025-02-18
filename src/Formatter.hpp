#pragma once

#include <format>

#include "Event.hpp"

//======================================

template<>
struct std::formatter<const Event&>: std::formatter<std::string>
{
	auto format(const Event& event, format_context ctx) const
	{
		return std::format("Sosi jopu");
	}
};

//======================================