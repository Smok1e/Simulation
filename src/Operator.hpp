#pragma once

#include <iostream>

#include "Transaction.hpp"
#include "EscapeSequence.hpp"

//======================================

class Operator
{
public:
	explicit Operator(size_t index);
	virtual ~Operator();

	bool processTransaction(Transaction transaction);
	bool isFree() const;
	int getProcessingTime() const;

	int getLoadTime() const;
	Transaction getCurrentTransaction() const;

	std::ostream& displayDetails(std::ostream& stream) const;
	friend std::ostream& operator<<(std::ostream& stream, const Operator& op);

protected:
	size_t m_index;
	Transaction m_current_transaction = Transaction::None;
	int m_processing_time = 0;
	int m_load_time = 0;

	virtual const char* getName() const = 0;
	virtual TermColor::Color getColor() const = 0;

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

protected:
	const char* getName() const override;
	TermColor::Color getColor() const override;
	int generateTransactionProcessingTime(Transaction transaction) const override;

};

class Operator2: public Operator
{
public:
	using Operator::Operator;

protected:
	const char* getName() const override;
	TermColor::Color getColor() const override;
	int generateTransactionProcessingTime(Transaction transaction) const override;

};

//======================================