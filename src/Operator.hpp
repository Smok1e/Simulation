#pragma once

#include <iostream>

#include "Transaction.hpp"
#include "EscapeSequence.hpp"

//======================================

class Operator
{
public:
	Operator() = default;
	virtual ~Operator();

	bool isFree() const;
	bool processTransaction(Transaction transaction);
	void onTimeTick();

	unsigned getLoadTicks() const;

	friend std::ostream& operator<<(std::ostream& stream, const Operator& op);

	// Displays processing transaction progress
	std::ostream& displayDetails(std::ostream& stream) const;

protected:
	virtual const char* getName() const = 0;
	virtual TermColor::Color getColor() const = 0;

	Transaction m_current_transaction = Transaction::None;
	int m_processing_time = 0;
	int m_remaining_processing_time = 0;
	int m_load_ticks = 0;

	// Returns -1 if unable to process this type of transaction
	virtual int generateTransactionProcessingTime(Transaction transaction) const = 0;

};

//======================================

// Operator requirement
template<class T>
concept IsOperator = requires(T op)
{
	std::is_base_of<Operator, T>::value;
};

//======================================

class Operator1: public Operator
{
public:
	using Operator::Operator;

	const char* getName() const override;
	TermColor::Color getColor() const override;
	int generateTransactionProcessingTime(Transaction transaction) const override;

};

class Operator2: public Operator
{
public:
	using Operator::Operator;

	const char* getName() const override;
	TermColor::Color getColor() const override;
	int generateTransactionProcessingTime(Transaction transaction) const override;

};

//======================================