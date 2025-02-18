#pragma once

#include <iostream>
#include "Transaction.hpp"
#include "Operator.hpp"

//======================================

class Operator
{
public:
	enum class Type
	{
		Operator1,
		Operator2,

		Amount
	};

	explicit Operator(Type type, size_t index);

	bool canProcessTransaction(Transaction transaction);
	void processTransaction(Transaction transaction);

	Type getType() const;
	bool isFree() const;
	int getLoadTime() const;
	int getProcessingTime() const;
	Transaction getCurrentTransaction() const;

	friend TerminalStream& operator<<(TerminalStream& stream, const Operator& op);
	void printStatus(TerminalStream& stream);

protected:
	size_t m_index;
	Type m_type;
	Transaction m_current_transaction = Transaction::None;

	int m_processing_time = 0;
	int m_load_time = 0;

};

//======================================