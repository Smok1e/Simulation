#include "Terminal.hpp"

//======================================

// Escape sequence helper constants
namespace esc
{
	const char* begin              = "\x1B[";
	const char* alt_buffer_enable  = "?1049h";
	const char* alt_buffer_disable = "?1049l";
	const char* clear              = "2J";
	const char* hide_cursor        = "?25l";

	unsigned bgcolor_offset = 40;
	unsigned fgcolor_offset = 30;
} // namespace esc

//======================================

#ifdef SIMULATION_WINDOWS

#define NOMINMAX
#include <Windows.h>

std::pair<size_t, size_t> GetTerminalWindowSize()
{
	CONSOLE_SCREEN_BUFFER_INFO info = {};
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
		throw std::runtime_error("unable to get terminal size info");

	return {info.dwSize.X, info.dwSize.Y};
}

#else

#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>

std::pair<size_t, size_t> GetTerminalWindowSize()
{
	winsize size = {};
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

	return {size.ws_col, size.ws_row};
}

#endif // SIMULATION_WINDOWS

//======================================

bool Terminal::Character::operator==(const Character& other) const
{
	return 
		code == other.code && 
		background == other.background && 
		foreground == other.foreground;
}

//======================================

Terminal::Terminal(bool restore /*= true*/, std::ostream* stream /*= &std::cout*/):
	m_stream(stream),
	m_restore(restore)
{
	auto size = GetTerminalWindowSize();
	m_width = size.first;
	m_height = size.second;

	if (m_restore)
		*m_stream << esc::begin << esc::alt_buffer_enable;

	setBackgroundColor(Color::Default, true);
	setForegroundColor(Color::Default, true);

	*m_stream << esc::begin << esc::clear;

	setCursorPosition(0, 0);
	*m_stream << esc::begin << esc::hide_cursor;

	m_buffer.resize(m_width * m_height);
	m_back_buffer = m_buffer;
}

Terminal::~Terminal()
{
	if (m_restore)
		*m_stream << esc::begin << esc::alt_buffer_disable;
}

//======================================

size_t Terminal::getWidth() const
{
	return m_width;
}

size_t Terminal::getHeight() const
{
	return m_height;
}

//======================================

void Terminal::display(bool force_redraw /*= false*/)
{
	for (size_t i = 0; i < m_width * m_height; i++)
	{
		if (m_buffer[i] == m_back_buffer[i])
			continue;

		setCursorPosition(i % m_width, i / m_width);

		auto& character = m_buffer[i];
		setBackgroundColor(character.background);
		setForegroundColor(character.foreground);
		m_stream->put(character.code);

		m_back_buffer[i] = character;
	}
	
	*m_stream << std::flush;
}

//======================================

void Terminal::set(
	size_t x,
	size_t y,
	char ch,
	Color background /*= Color::Default*/,
	Color foreground /*= Color::Default*/
)
{
	m_buffer[y * m_width + x] = {
		ch,
		background,
		foreground
	};
}

void Terminal::set(
	size_t x,
	size_t y,
	std::string_view str,
	Color background /*= Color::Default*/,
	Color foreground /*= Color::Default*/
)
{
	auto width = std::min<int>(str.length(), static_cast<int>(m_width) - x);

	for (size_t i = 0; i < width; i++)
		set(x + i, y, str[i], background, foreground);
}

void Terminal::clear(
	char ch /*= ' '*/,
	Color background /*= Color::Default*/,
	Color foreground /*= Color::Default*/
)
{
	Character character = {
		ch,
		background,
		foreground
	};

	std::fill(
		m_buffer.begin(),
		m_buffer.end(),
		character
	);
}

//======================================

void Terminal::setBackgroundColor(Color color, bool force /*= false*/)
{
	if (m_background_color == color && !force)
		return;

	*m_stream << esc::begin << esc::bgcolor_offset + static_cast<unsigned>(color) << 'm';
	m_background_color = color;
}

void Terminal::setForegroundColor(Color color, bool force /*= false*/)
{
	if (m_foreground_color == color && !force)
		return;

	*m_stream << esc::begin << esc::fgcolor_offset + static_cast<unsigned>(color) << 'm';
	m_foreground_color = color;
}

void Terminal::setCursorPosition(unsigned x, unsigned y)
{
	*m_stream << esc::begin << 1 + y << ';' << 1 + x << 'H';
}

//======================================

TerminalStream::TerminalStream(Terminal* terminal):
	std::ostream(this),
	m_terminal(terminal)
{
	m_background_color_stack.push(Terminal::Color::Default);
	m_foreground_color_stack.push(Terminal::Color::Default);
}

void TerminalStream::setPosition(size_t x, size_t y)
{
	sync();
	m_x = x;
	m_y = y;
}

std::pair<size_t, size_t> TerminalStream::getPosition() const
{
	return {m_x, m_y};
}

void TerminalStream::setPadding(size_t padding)
{
	sync();
	m_x = m_padding = padding;
}

size_t TerminalStream::getPadding() const
{
	return m_padding;
}

void TerminalStream::pushBackground(Terminal::Color color)
{
	sync();
	m_background_color_stack.push(color);
}

void TerminalStream::popBackground()
{
	sync();
	m_background_color_stack.pop();
}

void TerminalStream::pushForeground(Terminal::Color color)
{
	sync();
	m_foreground_color_stack.push(color);	
}

void TerminalStream::popForeground()
{
	sync();
	m_foreground_color_stack.pop();
}

void TerminalStream::newLine(size_t count /*= 1*/)
{
	m_x = m_padding;
	m_y += count;
}

void TerminalStream::clear()
{
	sync();
	m_terminal->clear();
	m_x = m_padding;
	m_y = 0;
}

void TerminalStream::display()
{
	sync();
	m_terminal->display();
}

void TerminalStream::print(std::string_view str) 
{
	auto offset = str.find('\n');
	auto begin = str.begin();

	while (offset != std::string_view::npos)
	{
		m_terminal->set(
			m_x,
			m_y,							
			std::string_view(begin, str.begin() + offset),
			m_background_color_stack.top(),
			m_foreground_color_stack.top()
		);

		newLine();
		begin = str.begin() + offset + 1;
		offset = str.find('\n', offset + 1);
	}

	m_terminal->set(
		m_x,
		m_y,
		std::string_view(begin, str.end()),
		m_background_color_stack.top(),
		m_foreground_color_stack.top()
	);

	m_x += str.end() - begin;
}

std::stringbuf::int_type TerminalStream::sync()
{
	print(view());
	str("");

	return 0;
}

//======================================