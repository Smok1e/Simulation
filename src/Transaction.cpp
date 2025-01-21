#include "Config.hpp"
#include "Transaction.hpp"
#include "Distribution.hpp"

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
	}
}

std::ostream& operator<<(std::ostream& stream, Transaction transaction)
{
	if (transaction == Transaction::None)
		return stream << "[none]";

	return stream << '[' << static_cast<int>(transaction) + 1 << ']';
}

//======================================