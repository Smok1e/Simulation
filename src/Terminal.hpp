#pragma once

#include <vector>
#include <iostream>
#include <string_view>
#include <tuple>
#include <sstream>
#include <stack>
#include <functional>

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

	Terminal(bool restore = true, std::ostream* stream = &std::cout);
	~Terminal();

	size_t getWidth() const;
	size_t getHeight() const;

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
	bool m_restore;

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

// TerminalStream acts as a stream buffer with
// public ostream interface, making it possible to use specific
// TerminalStream abilities along with standard output operations
class TerminalStream: private std::stringbuf, public std::ostream
{
public:
	TerminalStream(Terminal* terminal);

	void setPosition(size_t x, size_t y);
	std::pair<size_t, size_t> getPosition() const;

	void setPadding(size_t padding);
	size_t getPadding() const;

	void pushBackground(Terminal::Color color);
	void popBackground();

	void pushForeground(Terminal::Color color);
	void popForeground();

	void newLine(size_t count = 1);
	void clear();
	void display();

	void print(std::string_view str);

private:
	Terminal* m_terminal;

	std::stack<Terminal::Color> m_background_color_stack = {};
	std::stack<Terminal::Color> m_foreground_color_stack = {};

	size_t m_x = 0;
	size_t m_y = 0;
	size_t m_padding = 0;

	std::streambuf::int_type sync() override;

};

//======================================