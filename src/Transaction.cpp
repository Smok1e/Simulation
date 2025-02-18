#include <stdexcept>

#include "Config.hpp"
#include "Transaction.hpp"
#include "Distribution.hpp"

//======================================

int GenerateTransactionDelay(Transaction transaction)
{
	return ExponentialDistribution(TRANSACTION_AVG_DELAY.at(transaction));
}

TerminalStream& operator<<(TerminalStream& stream, Transaction transaction)
{
	switch (transaction)
	{
		case Transaction::Transaction1:
			stream.pushForeground(Terminal::Color::Cyan);
			break;

		case Transaction::Transaction2:
			stream.pushForeground(Terminal::Color::Red);
			break;

		case Transaction::Transaction3:
			stream.pushForeground(Terminal::Color::Yellow);
			break;
	}

	stream << static_cast<int>(transaction) + 1;
	stream.popForeground();

	return stream;
}

//======================================
