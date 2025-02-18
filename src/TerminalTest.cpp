#include "Terminal.hpp"
#include <iomanip>

//======================================

int main()
{
	using Color = Terminal::Color;

	Terminal terminal(false);
	TerminalStream stream(&terminal);

	stream << "Pidor jopa, govno pizda (";
	stream.pushForeground(Color::Green);
	stream << "green pizda";
	stream.popForeground();
	stream << ")" << std::endl;
	stream.setPadding(4);
	stream << "Padded pizda (";
	stream.pushForeground(Color::Blue);
	stream << "padded blue pizda + " << 123 << ")" << std::endl;
	stream.popForeground();
	stream << "Padded wrapped pizda govna (" << std::setw(10) << std::setfill('0') << 12345 <<")" << std::endl;

	stream.display();
	std::cin.get();
}

//======================================
