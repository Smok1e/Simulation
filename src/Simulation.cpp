#include <algorithm>
#include <iomanip>
#include <cmath>
#include <random>

#include "Distribution.hpp"
#include "Simulation.hpp"

//======================================

Simulation::Simulation(
	size_t op1_count, 
	size_t op2_count, 
	double max_avg_transaction_queued,
	size_t max_queue_size,
	bool   shuffle /*= false*/
):
	m_op1_count(op1_count),
	m_op2_count(op2_count),
	m_target_avg_transaction_queued(max_avg_transaction_queued),
	m_target_queue_size(max_queue_size)
{
	for (size_t i = 0; i < op1_count; i++)
		m_operators.push_back(new Operator1(i));

	for (size_t i = 0; i < op2_count; i++)
		m_operators.push_back(new Operator2(i));

	if (shuffle)
	{
		std::mt19937 random_engine(RandomSeed);
		std::shuffle(m_operators.begin(), m_operators.end(), random_engine);
	}

	// Start sequence of transaction income events for each transaction type
	for (size_t i = 0; i < static_cast<size_t>(Transaction::Amount); i++)
		enqueueEvent(new TransactionIncomeEvent(m_current_time, static_cast<Transaction>(i)));
}

Simulation::~Simulation()
{
	for (auto* event: m_event_queue)
		delete event;

	for (auto* op: m_operators)
		delete op;
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
		if (op->processTransaction(transaction))
		{
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
			if (op->processTransaction(transaction))
			{
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

	// Updating statistics
	for (const auto& transaction: m_transaction_queue)
		m_total_queued_transactions[static_cast<int>(transaction)]++;

	m_max_queue_size = std::max<int>(m_max_queue_size, m_transaction_queue.size());
}

//======================================

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

//======================================

void Simulation::displayStatistics(
	std::ostream& stream /*= std::cout*/,
	uint8_t options /*= DisplayOptions::Default*/
)
{
	if (options & DisplayOptions::SimulationTime)
	{
		stream << "Random seed: " << RandomSeed << std::endl;
		stream << "Simulation time: " << m_current_time << std::endl << std::endl;
	}

	// Pending events
	if (options & DisplayOptions::EventQueue)
	{
		stream << "Event queue:" << std::endl;

		if (!m_event_queue.empty())
		{
			int time_digits = std::floor(1 + std::log10((*m_event_queue.rbegin())->getTime()));

			for (auto* event: m_event_queue)
				std::cout << std::setw(time_digits) << event->getTime() << ": " << *event << std::endl;

			stream << std::endl;
		}
	}

	// Queue
	if (options & DisplayOptions::Queue)
	{
		stream << "Queue (" << m_transaction_queue.size() << "): ";	  
		if (m_transaction_queue.empty())
			stream << "[empty]";

		else
			for (auto transaction: m_transaction_queue)
				stream << transaction << ' ';

		stream << std::endl << std::endl;
	}

	// Operators
	if (options & DisplayOptions::Operators)
	{
		stream << "Operators:" << std::endl;

		if (options & DisplayOptions::OperatorsDetails)
			for (size_t i = 0; i < m_operators.size(); i++)
				m_operators[i]->displayDetails(stream) << std::endl;

		else
			stream 
				<< m_op1_count << " operators of type 1" << std::endl
				<< m_op2_count << " operators of type 2" << std::endl;

		stream << std::endl;
	}

	// Statistics
	if (options & DisplayOptions::Statistics)
	{
		// Average queued transactions
		for (size_t i = 0; i < static_cast<int>(Transaction::Amount); i++)
		{
			auto avg = getAverageQueuedTransactions(static_cast<Transaction>(i));

			stream 
				<< "Average queued transactions of type " << static_cast<Transaction>(i) << ": ";

			if (m_current_time == 0)
				stream << "N/A" << std::endl;

			else
				stream
					<< TermColor::Push(
						avg <= m_target_avg_transaction_queued
							? TermColor::ForegroundGreen
							: TermColor::ForegroundRed
					)
					<< std::fixed << std::setprecision(2) << avg
					<< " / " << m_target_avg_transaction_queued
					<< TermColor::Pop()
					<< std::endl;
		}

		stream << std::endl;

		// Average operator load
		for (size_t i = 0; i < m_operators.size(); i++)
		{
			stream
				<< *m_operators[i] << " average load: ";

			if (m_current_time == 0)
				stream << "N/A" << std::endl;

			else
				stream
					<< std::fixed << std::setprecision(2) << static_cast<double>(m_operators[i]->getLoadTime()) / m_current_time
					<< std::endl;
		}
		
		stream << std::endl;

		stream 
			<< "Maximum queue size: " 
			<< TermColor::Push(
				m_max_queue_size <= m_target_queue_size
					? TermColor::ForegroundGreen
					: TermColor::ForegroundRed
			) 
			<< m_max_queue_size 
			<< " / " << m_target_queue_size
			<< TermColor::Pop() 
			<< std::endl;
	}
}

//======================================

bool Simulation::checkTargetRequirements() const
{
	if (m_max_queue_size > m_target_queue_size)
		return false;

	for (size_t i = 0; i < static_cast<int>(Transaction::Amount); i++)
		if (getAverageQueuedTransactions(static_cast<Transaction>(i)) > m_target_avg_transaction_queued)
			return false;

	return true;
}

Simulation::operator bool() const
{
	return checkTargetRequirements();
}

//======================================
