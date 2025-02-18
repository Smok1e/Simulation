#pragma once

#include <queue>

#include "Transaction.hpp"
#include "Operator.hpp"
#include "Terminal.hpp"

//======================================

class Simulation;

class Event
{
public:
	Event(size_t time);

	size_t getTime() const;
	virtual void process(Simulation* simulation) = 0;

	friend TerminalStream& operator<<(TerminalStream& stream, const Event& event);

protected:
	virtual TerminalStream& serialize(TerminalStream& stream) const = 0;

	size_t m_time;

};

//======================================

struct EventPtrCmp
{
	bool operator()(const Event* a, const Event* b) const;
};

//======================================

class TransactionIncomeEvent: public Event
{
public:
	TransactionIncomeEvent(size_t current_time, Transaction transaction);

	void process(Simulation* simulation);

private:
	TerminalStream& serialize(TerminalStream& stream) const;

	Transaction m_transaction;

};

//======================================

class TransactionProcessedEvent: public Event
{
public:
	TransactionProcessedEvent(size_t current_time, Operator* op);

	void process(Simulation* simulation);

private:
	TerminalStream& serialize(TerminalStream& stream) const;

	Operator* m_operator;

};

//======================================