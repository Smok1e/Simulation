#include <algorithm>
#include <iomanip>
#include <cmath>
#include <random>

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
		m_operators.push_back(new Operator1);

	for (size_t i = 0; i < op2_count; i++)
		m_operators.push_back(new Operator2);

	if (shuffle)
	{
		std::random_device device;
		std::mt19937 generator(device());
		std::shuffle(m_operators.begin(), m_operators.end(), generator);
	}
}

Simulation::~Simulation()
{
	for (auto* op: m_operators)
		delete op;
}

//======================================

void Simulation::onTimeTick()
{
	m_time++;

	// Generating future transactions
	for (size_t i = 0; i < std::size(m_pending_transactions); i++)
	{
		if (!m_pending_transactions[i])
		{
			auto transaction = static_cast<Transaction>(i);

			m_queue.push_back(transaction);
			m_pending_transactions[i] = GenerateTransactionDelay(transaction);
		}

		else
			m_pending_transactions[i]--;
	}

	// Processing current transactions
	for (int i = 0; i < m_queue.size(); i++)
	{
		for (auto* op: m_operators)
		{
			if (op->processTransaction(m_queue[i]))
			{
				m_queue.erase(m_queue.begin() + i--);
				break;
			}
		}
	}

	for (auto* op: m_operators)
		op->onTimeTick();

	// Updating statistics
	for (const auto& transaction: m_queue)
		m_total_queued_transactions[static_cast<int>(transaction)]++;

	m_max_queue_size = std::max<int>(m_max_queue_size, m_queue.size());
}

//======================================

double Simulation::getAverageQueuedTransactions(Transaction transaction) const
{
	return static_cast<double>(
		m_total_queued_transactions[static_cast<int>(transaction)]
	) / m_time;
}

int Simulation::getMaxQueueSize() const
{
	return m_max_queue_size;
}

int Simulation::getTime() const
{
	return m_time;
}

//======================================

void Simulation::displayStatistics(
	std::ostream& stream /*= std::cout*/,
	uint8_t options /*= DisplayOptions::Default*/
)
{
	if (options & DisplayOptions::SimulationTime)
		stream << "Simulation time: " << m_time << std::endl << std::endl;

	// Queue
	if (options & DisplayOptions::Queue)
	{
		stream << "Queue (" << m_queue.size() << "): ";	  
		if (m_queue.empty())
			stream << "[empty]";

		else
			for (auto transaction: m_queue)
				stream << transaction << ' ';

		stream << std::endl << std::endl;
	}

	// Pending transactions
	if (options & DisplayOptions::PendingTransactions)
	{
		stream << "Transactions:" << std::endl;
		for (int i = 0; i < static_cast<int>(Transaction::Amount); i++)
			stream << "Type " << static_cast<Transaction>(i) << " pending in " << m_pending_transactions[i] << std::endl;

		stream << std::endl;
	}

	int operator_number_max_length = std::floor(1 + std::log10(m_operators.size()));

	// Operators
	if (options & DisplayOptions::Operators)
	{
		stream << "Operators:" << std::endl;

		if (options & DisplayOptions::OperatorsDetails)
			for (size_t i = 0; i < m_operators.size(); i++)
				m_operators[i]->displayDetails(stream << "[" << std::setw(operator_number_max_length) << i + 1 << "] - ") << std::endl;

		else
		{
			std::cout << m_op1_count << " operators of type 1" << std::endl;
			std::cout << m_op2_count << " operators of type 2" << std::endl;
		}

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
				<< "Average queued transactions of type " << static_cast<Transaction>(i) << ": " 
				<< TermColor::Push(
					avg <= m_target_avg_transaction_queued
						? TermColor::ForegroundGreen
						: TermColor::ForegroundRed
				)
				<< std::fixed << std::setprecision(2) << avg
				<< TermColor::Pop()
				<< std::endl;
		}

		stream << std::endl;

		// Average operator load
		for (size_t i = 0; i < m_operators.size(); i++)
		{
			stream
				<< "Operator #" << std::setw(operator_number_max_length) << std::left << i + 1 
				<< " (" << *m_operators[i] << ") average load: " 
				<< std::fixed << std::setprecision(2) << static_cast<double>(m_operators[i]->getLoadTicks()) / m_time
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
