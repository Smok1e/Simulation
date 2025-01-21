#include <stdexcept>

#include "Config.hpp"
#include "Distribution.hpp"
#include "Operator.hpp"

//======================================

Operator::~Operator()
{}

bool Operator::isFree() const
{
	return !m_remaining_processing_time;
}

bool Operator::processTransaction(Transaction transaction)
{
	if (!isFree())
		return false;

	auto processing_time = generateTransactionProcessingTime(transaction);
	if (processing_time < 0)
		return false;

	m_current_transaction = transaction;
	m_remaining_processing_time = processing_time;

	return true;
}

void Operator::onTimeTick()
{
	if (!(m_remaining_processing_time -= !!m_remaining_processing_time))
		m_current_transaction = Transaction::None;
}

std::ostream& operator<<(std::ostream& stream, const Operator& op)
{
	stream << op.getName();

	if (op.isFree())
		stream << " (free)";

	else
		stream 
		<< " (processing " 
		<< op.m_current_transaction << " for ahother " 
		<< std::setw(2) << op.m_remaining_processing_time 
		<< "s)";

	return stream;
}

//======================================

const char* Operator1::getName() const
{
	return "Operator1";
}

int Operator1::generateTransactionProcessingTime(Transaction transaction) const
{
	switch (transaction)
	{
		case Transaction::Transaction1:
			return ExponentialDistribution(TRANSACTION1_AVG_PROCESSING_TIME);

		case Transaction::Transaction3:
			return ExponentialDistribution(TRANSACTION3_AVG_PROCESSING_TIME_OP1);

		default:
			return -1;
	}
}

//======================================

const char* Operator2::getName() const
{
	return "Operator2";
}

int Operator2::generateTransactionProcessingTime(Transaction transaction) const
{
	switch (transaction)
	{
		case Transaction::Transaction2:
			return ExponentialDistribution(TRANSACTION2_AVG_PROCESSING_TIME);

		case Transaction::Transaction3:
			return ExponentialDistribution(TRANSACTION3_AVG_PROCESSING_TIME_OP2);

		default:
			return -1;
	}
}

//======================================