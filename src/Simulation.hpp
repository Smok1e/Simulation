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
	Simulation(size_t op1_count, size_t op2_count, bool shuffle = false);
	~Simulation();

	void onTimeTick();

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

protected:
	std::vector<Operator*> m_operators {};
	std::vector<Transaction> m_queue {};

	int m_pending_transactions[static_cast<int>(Transaction::Amount)] {};

	int m_total_queued_transactions[static_cast<int>(Transaction::Amount)] {};
	int m_max_queue_size = 0;
	int m_time = 0;

	size_t m_op1_count;
	size_t m_op2_count;

};

//======================================
