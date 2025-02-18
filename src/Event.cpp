#include "Event.hpp"				`
#include "Simulation.hpp"

//======================================

Event::Event(size_t time):
	m_time(time)
{}

size_t Event::getTime() const
{
	return m_time;
}

TerminalStream& operator<<(TerminalStream& stream, const Event& event)
{
	return event.serialize(stream);
}

//======================================

bool EventPtrCmp::operator()(const Event* a, const Event* b) const
{
	return a->getTime() < b->getTime();
}

//======================================

TransactionIncomeEvent::TransactionIncomeEvent(size_t current_time, Transaction transaction):
	Event(current_time + GenerateTransactionDelay(transaction)),
	m_transaction(transaction)
{}

void TransactionIncomeEvent::process(Simulation* simulation)
{
	simulation->enqueueTransaction(m_transaction);
	simulation->enqueueEvent(
		new TransactionIncomeEvent(
			simulation->getCurrentTime(), 
			m_transaction
		)
	);
}

TerminalStream& TransactionIncomeEvent::serialize(TerminalStream& stream) const
{
	stream << "Transaction ";
	stream << m_transaction;
	stream << " pending";

	return stream;
}

//======================================

TransactionProcessedEvent::TransactionProcessedEvent(size_t current_time, Operator* op):
	Event(current_time + op->getProcessingTime()),
	m_operator(op)
{}

void TransactionProcessedEvent::process(Simulation* simulation)
{
	m_operator->processTransaction(Transaction::None);
}

TerminalStream& TransactionProcessedEvent::serialize(TerminalStream& stream) const
{
	stream << "Transaction ";
	stream << m_operator->getCurrentTransaction();
	stream << " will be processed by ";
	stream << *m_operator;

	return stream;
}

//======================================