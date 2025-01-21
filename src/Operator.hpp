#pragma once

#include <iostream>

#include "Transaction.hpp"

//======================================

class Operator
{
public:
	Operator() = default;
	virtual ~Operator();

	bool isFree() const;
	bool processTransaction(Transaction transaction);
	void onTimeTick();

	friend std::ostream& operator<<(std::ostream& stream, const Operator& op);

protected:
	virtual const char* getName() const = 0;

	Transaction m_current_transaction = Transaction::None;
	int m_processing_time = 0;
	int m_remaining_processing_time = 0;

	// Returns -1 if unable to process this type of transaction
	virtual int generateTransactionProcessingTime(Transaction transaction) const = 0;

};

//======================================

class Operator1: public Operator
{
public:
	using Operator::Operator;

	const char* getName() const override;
	int generateTransactionProcessingTime(Transaction transaction) const override;

};

class Operator2: public Operator
{
public:
	using Operator::Operator;

	const char* getName() const override;
	int generateTransactionProcessingTime(Transaction transaction) const override;

};

//======================================
