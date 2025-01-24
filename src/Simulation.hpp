#pragma once

#include <vector>
#include <iostream>
#include <cstdint>

#include "Operator.hpp"
#include "Transaction.hpp"

//======================================

class Simulation
{
public:
	Simulation(
		size_t op1_count, 
		size_t op2_count, 
		double max_avg_transaction_queued,
		size_t max_queue_size,
		bool   shuffle = false
	);

	~Simulation();

	void onTimeTick();

	template<IsOperator T>
	double getAverageOperatorLoad();

	double getAverageQueuedTransactions(Transaction transaction) const;
	int getMaxQueueSize() const;
	int getTime() const;

	enum DisplayOptions
	{
		SimulationTime      = 0b00000001, // Display simulation time
		Queue               = 0b00000010, // Display transaction queue
		PendingTransactions = 0b00000100, // Display pending transactions time
		Operators           = 0b00001000, // Display operators summary
		OperatorsDetails    = 0b00010000, // Display every operator status
		Statistics          = 0b00100000, // Display simulation statistics

		Default = 
			SimulationTime | Queue | PendingTransactions | Operators | OperatorsDetails | Statistics
	};

	void displayStatistics(
		std::ostream& stream = std::cout, 
		uint8_t options = DisplayOptions::Default
	);

	bool checkTargetRequirements() const;
	operator bool() const;

protected:
	std::vector<Operator*> m_operators {};
	std::vector<Transaction> m_queue {};

	int m_pending_transactions[static_cast<int>(Transaction::Amount)] {};

	int m_total_queued_transactions[static_cast<int>(Transaction::Amount)] {};
	int m_max_queue_size = 0;
	int m_time = 0;

	double m_target_avg_transaction_queued;
	size_t m_target_queue_size;

	size_t m_op1_count;
	size_t m_op2_count;

};

//======================================

template<IsOperator T>
double Simulation::getAverageOperatorLoad()
{
	double load = 0;
	size_t count = 0;

	for (const auto* op: m_operators)
	{
		if (!dynamic_cast<const T*>(op))
			continue;

		load += op->getLoadTicks(), count++;
	}

	return load / (count * getTime());
}

//======================================