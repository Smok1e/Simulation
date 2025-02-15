#pragma once

#include <vector>
#include <iostream>
#include <string_view>
#include <tuple>

//======================================

class Terminal
{
public:
	enum class Color
	{
		Black,
		Red,
		Green,
		Yellow,
		Blue,
		Magenta,
		Cyan,
		White,

		Default = 9
	};

	Terminal(std::ostream* stream = &std::cout);

	size_t getWidth() const;
	size_t getHeight() const;

	void enableAltBuffer(bool enable);

	// Displays changed pixels into the specified stream
	void display(bool force_redraw = false);

	// Sets character at the given position
	void set(
		size_t x,
		size_t y,
		char ch,
		Color background = Color::Default,
		Color foreground = Color::Default
	);

	// Puts string at the given position
	void set(
		size_t x, 
		size_t y, 
		std::string_view str, 
		Color background = Color::Default, 
		Color foreground = Color::Default
	);

	// Fills entire buffer with specific character
	void clear(
		char ch = ' ',
		Color background = Color::Default,
		Color foreground = Color::Default
	);

private:
	struct Character
	{
		char code = ' ';
		Color background = Color::Default;
		Color foreground = Color::Default;

		bool operator==(const Character& other) const;
	};

	std::ostream* m_stream;

	std::vector<Character> m_buffer = {};
	std::vector<Character> m_back_buffer = {};

	size_t m_width = 0;
	size_t m_height = 0;

	Color m_background_color = Color::Default;
	Color m_foreground_color = Color::Default;

	void setBackgroundColor(Color color, bool force = false);
	void setForegroundColor(Color color, bool force = false);
	void setCursorPosition(unsigned x, unsigned y);

};

//======================================