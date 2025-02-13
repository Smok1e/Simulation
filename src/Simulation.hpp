#pragma once

#include <vector>
#include <iostream>
#include <set>
#include <cstdint>

#include "Operator.hpp"
#include "Transaction.hpp"
#include "Event.hpp"

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

	// Push event into queue
	void enqueueEvent(Event* event);

	// Try to process transaction immediately or push it into the queue
	void enqueueTransaction(Transaction transaction);

	// Find appropriate transaction for an operator
	bool loadOperator(Operator* op);

	// Advance simulation to the next event
	void advance();

	// Statistic calculation methods
	template<IsOperator T>
	double getAverageOperatorLoad();

	double getAverageQueuedTransactions(Transaction transaction) const;
	int getMaxQueueSize() const;
	int getCurrentTime() const;

	// Ensure that current system status fits into limits
	bool checkTargetRequirements() const;
	operator bool() const;

	// Output
	enum DisplayOptions
	{
		SimulationTime      = 0b00000001, // Display simulation time
		Queue               = 0b00000010, // Display transaction queue
		EventQueue = 0b00000100, // Display pending transactions time
		Operators           = 0b00001000, // Display operators summary
		OperatorsDetails    = 0b00010000, // Display every operator status
		Statistics          = 0b00100000, // Display simulation statistics

		Default = 
			SimulationTime | Queue | EventQueue | Operators | OperatorsDetails | Statistics
	};

	void displayStatistics(
		std::ostream& stream = std::cout, 
		uint8_t options = DisplayOptions::Default
	);

protected:
	std::multiset<Event*, EventPtrCmp> m_event_queue {};
	std::vector<Transaction> m_transaction_queue {};
	std::vector<Operator*> m_operators {};

	bool processTransaction(Transaction transaction);

	// Statistics
	int m_current_time = 0;
	int m_total_queued_transactions[static_cast<int>(Transaction::Amount)] {};
	int m_max_queue_size = 0;

	// Input
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

		load += op->getLoadTime(), count++;
	}

	return load / (count * getCurrentTime());
}

//======================================