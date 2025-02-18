#pragma once

#include "Terminal.hpp"

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

TerminalStream& operator<<(TerminalStream& stream, Transaction transaction);

//======================================