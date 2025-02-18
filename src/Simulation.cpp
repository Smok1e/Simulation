#include <algorithm>
#include <iomanip>
#include <cmath>
#include <random>

#include "Distribution.hpp"
#include "Simulation.hpp"

//======================================

Simulation::~Simulation()
{
	for (auto* event: m_event_queue)
		delete event;

	for (auto* op: m_operators)
		delete op;
}

//======================================

void Simulation::populate(size_t op1_count, size_t op2_count, bool shuffle)
{
	for (size_t i = 0; i < op1_count; i++)
		m_operators.push_back(new Operator(Operator::Type::Operator1, i));

	for (size_t i = 0; i < op2_count; i++)
		m_operators.push_back(new Operator(Operator::Type::Operator2, i));

	if (shuffle)
	{
		std::mt19937 generator(RandomSeed);
		std::shuffle(m_operators.begin(), m_operators.end(), generator);
	}
}

void Simulation::start()
{
	for (size_t i = 0; i < static_cast<size_t>(Transaction::Amount); i++)
		enqueueEvent(new TransactionIncomeEvent(getCurrentTime(), static_cast<Transaction>(i)));
}

//======================================

void Simulation::enqueueEvent(Event* event)
{
	m_event_queue.insert(event);
}

void Simulation::enqueueTransaction(Transaction transaction)
{
	if (!processTransaction(transaction))
		m_transaction_queue.push_back(transaction);
}

bool Simulation::processTransaction(Transaction transaction)
{
	for (auto* op: m_operators)
	{
		if (op->canProcessTransaction(transaction))
		{
			op->processTransaction(transaction);
			enqueueEvent(new TransactionProcessedEvent(m_current_time, op));
			return true;
		}
	}

	return false;
}

bool Simulation::loadOperator(Operator* op)
{
	auto iter = std::find_if(
		m_transaction_queue.begin(),
		m_transaction_queue.end(),
		[&](Transaction transaction) -> bool
		{
			if (op->canProcessTransaction(transaction))
			{
				op->processTransaction(transaction);
				enqueueEvent(new TransactionProcessedEvent(m_current_time, op));
				return true;
			}

			return false;
		}
	);

	if (iter == m_transaction_queue.end())
		return false;

	m_transaction_queue.erase(iter);
	return true;
}

//======================================

void Simulation::advance()
{
	if (m_event_queue.empty())
		throw std::runtime_error("No more events to process");

	auto* event = m_event_queue.extract(m_event_queue.begin()).value();

	m_current_time = event->getTime();
	event->process(this);
	delete event;

	for (auto* op: m_operators)
		if (op->isFree()) loadOperator(op);

	// Updating statistics
	for (const auto& transaction: m_transaction_queue)
		m_total_queued_transactions[static_cast<int>(transaction)]++;

	m_max_queue_size = std::max<int>(m_max_queue_size, m_transaction_queue.size());
}

//======================================

double Simulation::getAverageOperatorLoad(Operator::Type type) const
{
	size_t sum = 0, count = 0;

	for (const auto* op: m_operators)
		if (op->getType() == type)
			sum += op->getLoadTime(), count++;

	return (static_cast<double>(sum) / count) / m_current_time;
}

double Simulation::getAverageQueuedTransactions(Transaction transaction) const
{
	return static_cast<double>(
		m_total_queued_transactions[static_cast<int>(transaction)]
	) / m_current_time;
}

int Simulation::getMaxQueueSize() const
{
	return m_max_queue_size;
}

int Simulation::getCurrentTime() const
{
	return m_current_time;
}

std::multiset<Event*, EventPtrCmp>& Simulation::getEventQueue()
{
	return m_event_queue;
}

std::vector<Transaction>& Simulation::getTransactionQueue()
{
	return m_transaction_queue;
}

std::vector<Operator*>& Simulation::getOperators()
{
	return m_operators;
}

//======================================