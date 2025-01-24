#pragma once

#include <iostream>
#include <stack>

//======================================

class EscapeSequence
{
public:
	EscapeSequence() = default;

	friend std::ostream& operator<<(std::ostream& stream, const EscapeSequence& sequence);

protected:
	virtual std::ostream& serialize(std::ostream& stream) const = 0; 

};

//======================================

class TermColor: public EscapeSequence
{
public:
	enum Color
	{
		ForegroundBlack = 30,
		ForegroundRed,
		ForegroundGreen,
		ForegroundYellow,
		ForegroundBlue,
		ForegroundMagenta,
		ForegroundCyan,
		ForegroundWhite,
		ForegroundDefault = 39,

		BackgroundBlack = 40,
		BackgroundRed,
		BackgroundGreen,
		BackgroundYellow,
		BackgroundBlue,
		BackgroundMagenta,
		BackgroundCyan,
		BackgroundWhite,
		BackgroundDefault = 49
	};

	TermColor(Color color = ForegroundDefault);

	static TermColor Push(Color color);
	static TermColor Pop();

protected:
	static std::stack<Color> s_stack;

	Color m_color;

	std::ostream& serialize(std::ostream& stream) const;

};

std::ostream& operator<<(std::ostream& stream, TermColor::Color color);

//======================================

class TermClear: public EscapeSequence
{
public:
	using EscapeSequence::EscapeSequence;

protected:
	std::ostream& serialize(std::ostream& stream) const;

};

//======================================

class TermCursorPos: public EscapeSequence
{
public:
	TermCursorPos(unsigned x, unsigned y);

protected:
	unsigned m_x, m_y;

	std::ostream& serialize(std::ostream& stream) const;

};

//======================================