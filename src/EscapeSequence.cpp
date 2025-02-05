#include "EscapeSequence.hpp"

//======================================

std::ostream& operator<<(std::ostream& stream, const EscapeSequenceBase& sequence)
{
	return sequence.serialize(stream << "\x1B[");
}

//======================================

EscapeSequence::EscapeSequence(const char* sequence):
	m_sequence(sequence)
{}

std::ostream& EscapeSequence::serialize(std::ostream& stream) const
{
	return stream << m_sequence;
}	

EscapeSequence EscapeSequence::Clear                    = "2J";
EscapeSequence EscapeSequence::EnableAlternativeBuffer  = "?1049h";
EscapeSequence EscapeSequence::DisableAlternativeBuffer = "?1049l";

//======================================

std::stack<TermColor::Color> TermColor::s_stack;

TermColor::TermColor(Color color /*= ForegroundDefault*/):
	m_color(color)
{}

std::ostream& TermColor::serialize(std::ostream& stream) const
{
	return stream << "1;34;" << static_cast<int>(m_color) << 'm';
}

std::ostream& operator<<(std::ostream& stream, TermColor::Color color)
{
	return stream << TermColor(color);
}

TermColor TermColor::Push(TermColor::Color color)
{
	s_stack.push(color);
	return TermColor(color);
}

TermColor TermColor::Pop()
{
	s_stack.pop();
	return TermColor(s_stack.top());
}

//======================================

TermCursorPos::TermCursorPos(unsigned x, unsigned y):
	m_x(x),
	m_y(y)
{}

std::ostream& TermCursorPos::serialize(std::ostream& stream) const
{
	return stream << m_y + 1 << ';' << m_x + 1 << 'H';
}

//======================================
