#pragma once

#include <map>

#include "Transaction.hpp"
#include "Operator.hpp"

//======================================

constexpr int  DEFAULT_SIMULATION_DURATION   = 1000;
constexpr int  DEFAULT_AVG_TRANSACTION_QUEUE = 3;
constexpr int  DEFAULT_MAX_QUEUE_SIZE        = 7;
constexpr int  DEFAULT_DELAY_MS              = 100;
constexpr int  DEFAULT_OP1_COUNT             = 5;
constexpr int  DEFAULT_OP2_COUNT             = 5;

constexpr int R1 = 9, G1 =  7, B1 = 8;
constexpr int R2 = 6, G2 = 11, B2 = 6;
constexpr int R3 = 5, G3 =  7, B3 = 7;

constexpr auto DISPLAY_MAX_EVENT_QUEUE       = 20;
constexpr auto DISPLAY_MAX_TRANSACTION_QUEUE = 20;

//======================================

const std::map<Transaction, int> TRANSACTION_AVG_DELAY = {
	{Transaction::Transaction1, R1},
	{Transaction::Transaction2, G1},
	{Transaction::Transaction3, B1}
};

const std::map<Operator::Type, std::map<Transaction, int>> TRANSACTION_AVG_PROCESSING_TIME = {
	{
		Operator::Type::Operator1, {
			{Transaction::Transaction1, R1 + G1 + B1     },
			{Transaction::Transaction3, R3 + G3 + B3 + G1}
		}
	},
	{
		Operator::Type::Operator2, {
			{Transaction::Transaction2, R2 + G2 + B2     },
			{Transaction::Transaction3, R3 + G3 + B3 + B1}
		}
	}
};						

//======================================
