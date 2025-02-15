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

#error "слыш"

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

Terminal::Terminal(std::ostream* stream /*= &std::cout*/):
	m_stream(stream)
{
	auto size = GetTerminalWindowSize();
	m_width = size.first;
	m_height = size.second;

	setBackgroundColor(Color::Default, true);
	setForegroundColor(Color::Default, true);

	*m_stream 
		<< esc::begin << esc::clear 
		<< esc::begin << esc::hide_cursor;

	setCursorPosition(0, 0);

	m_buffer.resize(m_width * m_height);
	m_back_buffer = m_buffer;
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

void Terminal::enableAltBuffer(bool enable)
{
	*m_stream << esc::begin << (
		enable
			? esc::alt_buffer_enable
			: esc::alt_buffer_disable
	);
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