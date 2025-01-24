#include <stdexcept>

#include "Config.hpp"
#include "Transaction.hpp"
#include "Distribution.hpp"
#include "EscapeSequence.hpp"

//======================================

int GenerateTransactionDelay(Transaction transaction)
{
	switch (transaction)
	{
		case Transaction::Transaction1:
			return ExponentialDistribution(TRANSACTION1_AVG_DELAY);

		case Transaction::Transaction2:
			return ExponentialDistribution(TRANSACTION2_AVG_DELAY);

		case Transaction::Transaction3:
			return ExponentialDistribution(TRANSACTION3_AVG_DELAY);

		default:
			throw std::runtime_error("unknown transaction type");
	}
}

TermColor::Color TransactionColor(Transaction transaction)
{
	switch (transaction)
	{
		case Transaction::Transaction1:
			return TermColor::ForegroundCyan;

		case Transaction::Transaction2:
			return TermColor::ForegroundRed;

		case Transaction::Transaction3:
			return TermColor::ForegroundYellow;

		default:
			return TermColor::ForegroundDefault;
	}
}

std::ostream& operator<<(std::ostream& stream, Transaction transaction)
{
	if (transaction == Transaction::None)
		return stream << "[none]";

	return stream 
		<< TermColor::Push(TransactionColor(transaction)) 
		<< static_cast<int>(transaction) + 1 
		<< TermColor::Pop();
}

//======================================
