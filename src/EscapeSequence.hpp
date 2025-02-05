#pragma once

#include <iostream>
#include <stack>

//======================================

class EscapeSequenceBase
{
public:
	EscapeSequenceBase() = default;

	friend std::ostream& operator<<(std::ostream& stream, const EscapeSequenceBase& sequence);

protected:
	virtual std::ostream& serialize(std::ostream& stream) const = 0;

};

//======================================

// Simple sequences
class EscapeSequence: public EscapeSequenceBase
{
public:
	EscapeSequence(const char* sequence);

    static EscapeSequence Clear;
	static EscapeSequence EnableAlternativeBuffer;
	static EscapeSequence DisableAlternativeBuffer;

private:
	std::ostream& serialize(std::ostream& stream) const override;

	const char* m_sequence;

};

//======================================

class TermColor: public EscapeSequenceBase
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

	std::ostream& serialize(std::ostream& stream) const override;

};

std::ostream& operator<<(std::ostream& stream, TermColor::Color color);

//======================================

class TermCursorPos: public EscapeSequenceBase
{
public:
	TermCursorPos(unsigned x, unsigned y);

protected:
	unsigned m_x, m_y;

	std::ostream& serialize(std::ostream& stream) const override;

};

//======================================
