#include <iomanip>

#include "Config.hpp"
#include "Distribution.hpp"
#include "Operator.hpp"

//======================================

Operator::Operator(Operator::Type type, size_t index):
	m_type(type),
	m_index(index)
{}

//======================================

bool Operator::canProcessTransaction(Transaction transaction)
{
	return TRANSACTION_AVG_PROCESSING_TIME.at(m_type).contains(transaction) && isFree();
}

void Operator::processTransaction(Transaction transaction)
{
	if (transaction == Transaction::None)
	{
		m_load_time += m_processing_time;

		m_processing_time = 0;
		m_current_transaction = transaction;
		return;
	}

	if (!isFree())
		throw std::runtime_error("operator is busy");

	auto processing_time = ExponentialDistribution(
		TRANSACTION_AVG_PROCESSING_TIME.at(m_type).at(transaction)
	);

	m_processing_time = processing_time;
	m_current_transaction = transaction;
}

//======================================

Operator::Type Operator::getType() const
{
	return m_type;
}

bool Operator::isFree() const
{
	return m_current_transaction == Transaction::None;
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

//======================================

TerminalStream& operator<<(TerminalStream& stream, const Operator& op)
{
	switch (op.m_type)
	{
		case Operator::Type::Operator1:
			stream.pushForeground(Terminal::Color::Cyan);
			break;

		case Operator::Type::Operator2:
			stream.pushForeground(Terminal::Color::Red);
			break;
	}

	stream << "Operator" << static_cast<int>(op.m_type);
	stream.popForeground();

	stream << " #" << std::setw(2) << std::left << op.m_index + 1;

	return stream;
}

void Operator::printStatus(TerminalStream& stream)
{
	if (m_current_transaction == Transaction::None)
	{
		stream.pushForeground(Terminal::Color::Green);
		stream << "Free";
		stream.popForeground();
	}

	else
	{
		stream.pushForeground(Terminal::Color::Blue);
		stream << "Processing transaction "; 
		stream << m_current_transaction;
		stream.popForeground();
	}
}

//======================================