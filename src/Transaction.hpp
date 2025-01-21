#pragma once

#include <iostream>

//======================================

enum class Transaction
{
	Transaction1,
	Transaction2,
	Transaction3,

	Amount,
	None
};

int GenerateTransactionDelay(Transaction transaction);
std::ostream& operator<<(std::ostream& stream, Transaction transaction);

//======================================