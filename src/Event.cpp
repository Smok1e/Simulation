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

std::ostream& operator<<(std::ostream& stream, const Event& event)
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

std::ostream& TransactionIncomeEvent::serialize(std::ostream& stream) const
{
	return stream << "Transaction " << m_transaction << " pending";
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

std::ostream& TransactionProcessedEvent::serialize(std::ostream& stream) const
{
	return stream << *m_operator << " will complete processing transaction " << m_operator->getCurrentTransaction();
}

//======================================