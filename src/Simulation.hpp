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
	Simulation() = default;
	~Simulation();

	// Generate certain amount of operators
	void populate(size_t op1_count, size_t op2_count, bool shuffle);

	// Generate initial sequence of transaction pending events
	void start();

	// Push event into queue
	void enqueueEvent(Event* event);

	// Try to process transaction immediately or push it into the queue
	void enqueueTransaction(Transaction transaction);

	// Find appropriate transaction for an operator
	bool loadOperator(Operator* op);

	// Advance simulation to the next event
	void advance();

	// Statistic calculation methods
	double getAverageOperatorLoad(Operator::Type type) const;
	double getAverageQueuedTransactions(Transaction transaction) const;
	int getMaxQueueSize() const;
	int getCurrentTime() const;

	std::multiset<Event*, EventPtrCmp>& getEventQueue();
	std::vector<Transaction>& getTransactionQueue();
	std::vector<Operator*>& getOperators();

protected:
	std::multiset<Event*, EventPtrCmp> m_event_queue {};
	std::vector<Transaction> m_transaction_queue {};
	std::vector<Operator*> m_operators {};

	bool processTransaction(Transaction transaction);

	// Statistics
	int m_current_time = 0;
	int m_total_queued_transactions[static_cast<int>(Transaction::Amount)] {};
	int m_max_queue_size = 0;

};

//======================================
