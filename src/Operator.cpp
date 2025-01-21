#include <iomanip>

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
	m_remaining_processing_time = m_processing_time = processing_time;

	return true;
}

void Operator::onTimeTick()
{
	if (!(m_remaining_processing_time -= !!m_remaining_processing_time))
		m_current_transaction = Transaction::None;
}

std::ostream& operator<<(std::ostream& stream, const Operator& op)
{
	stream << op.getName() << ": ";

	if (op.isFree())
		stream << "Free";

	else
	{
        stream 
			<< "Processing " << op.m_current_transaction << ' '
			<< std::setw(3) << op.m_processing_time - op.m_remaining_processing_time << " / "
			<< std::setw(3) << op.m_processing_time << "s ";

		constexpr size_t max_width = 30;
		size_t width = max_width * static_cast<double>(op.m_remaining_processing_time) / op.m_processing_time;

		stream 
			<< '['
			<< std::setfill('=') << std::setw(max_width - width) << ">"
			<< std::setfill(' ') << std::setw(            width) << ""
			<< ']';
	}

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
