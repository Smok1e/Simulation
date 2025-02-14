#pragma once

#include <vector>

#include "EscapeSequence.hpp"

//======================================

class Terminal
{
public:
	struct Character
	{
		char ch = ' ';
		TermColor::Color background;
		TermColor::Color foreground;
	};

	Terminal(size_t width, size_t height);

	size_t getWidth() const;
	size_t getHeight() const;

private:
	size_t m_width;
	size_t m_height;

	std::vector<Character> m_buffer = {};
	std::vector<Character> m_back_buffer = {};

};

//======================================