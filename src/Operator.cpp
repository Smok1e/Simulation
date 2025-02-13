#include <iomanip>

#include "Config.hpp"
#include "Distribution.hpp"
#include "Operator.hpp"
#include "EscapeSequence.hpp"

//======================================

Operator::Operator(size_t index):
	m_index(index)
{}

Operator::~Operator()
{}

bool Operator::isFree() const
{
	return m_current_transaction == Transaction::None;
}

bool Operator::processTransaction(Transaction transaction)
{
	if (transaction == Transaction::None)
	{
		m_load_time += m_processing_time;
		m_current_transaction = transaction;
		return true;
	}

	if (!isFree())
		return false;

	auto processing_time = generateTransactionProcessingTime(transaction);
	if (processing_time < 0)
		return false;

	m_processing_time = processing_time;
	m_current_transaction = transaction;
	return true;
}

int Operator::getProcessingTime() const
{
	return m_processing_time;
}

int Operator::getLoadTime() const
{
	return m_load_time;
}

Transaction Operator::getCurrentTransaction() const
{
	return m_current_transaction;
}

std::ostream& Operator::displayDetails(std::ostream& stream) const
{
	stream << *this << ": ";

	if (isFree())
		stream 
			<< TermColor::Push(TermColor::ForegroundGreen) 
			<< "Free" 
			<< TermColor::Pop();

	else
	{
		stream
			<< TermColor::Push(TermColor::ForegroundBlue)
			<< "Processing " << m_current_transaction << "..."
			<< TermColor::Pop();
	}

	return stream;
}

std::ostream& operator<<(std::ostream& stream, const Operator& op)
{
	return stream 
		<< TermColor::Push(op.getColor()) 
		<< op.getName() << " [" << std::setw(2) << op.m_index << ']'
		<< TermColor::Pop(); 
}

//======================================

const char* Operator1::getName() const
{
	return "Operator1";
}

TermColor::Color Operator1::getColor() const
{
	return TermColor::ForegroundCyan;
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

TermColor::Color Operator2::getColor() const
{
	return TermColor::ForegroundRed;
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
