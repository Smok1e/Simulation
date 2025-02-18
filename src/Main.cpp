#include <iostream>
#include <chrono>
#include <string_view>
#include <cstring>
#include <cmath>

#include "Distribution.hpp"
#include "Config.hpp"
#include "Simulation.hpp"
#include "ArgParser.hpp"
#include "SignalHandler.hpp"
#include "Terminal.hpp"
#include "Formatter.hpp"

using namespace std::chrono_literals;

//======================================

enum DisplayOptions
{
	TransactionQueue = 0b00000001,
	Operators        = 0b00000010,
	OperatorsDetails = 0b00000100,
	Statistics       = 0b00001000,
	EventQueue       = 0b00010000,

	Interactive = TransactionQueue | Operators | OperatorsDetails | Statistics | EventQueue,
	Optimize = Statistics | Operators
};

void DisplaySimulationStatus(
	TerminalStream& stream,
	Simulation& simulation,
	DisplayOptions options = DisplayOptions::Interactive
);
							
//======================================

int main(int argc, char* argv[])
{
	try
	{
		ArgParser options {
			{"help",            "Print usage reference and exit"                             },
			{"version",         "Print build information and exit"                           },
			{"seed",            "Set random seed",                                       true},
			{"mode",            "Set simulation mode",                                   true},
			{"time",            "Set simulation duration (optimize mode)",               true},
			{"delay",           "Set delay in milliseconds",                             true},
			{"max-avg-tr", 'X', "Set target average transactions limit (optimize mode)", true},
			{"max-queue",  'Y', "Set target queue size limit (optimize mode)",           true},
			{"op1",        '1', "Set number of operators of type 1",                     true},
			{"op2",        '2', "Set number of operators of type 2",                     true},
			{"shuffle",    'S', "Enable operator shuffling"                                  }
		};
					 
		options.parse(argc, argv);

		// Display help and exit
		if (options["help"])
		{
			std::cout 
				<< "Usage: " << options.getExecutablePath().filename().string() << " [OPTIONS]" << std::endl
				<< std::endl
				<< "Available options:" << std::endl
				<< options << std::endl;

			return 0;
		}

		// Display version and exit
		if (options["version"])
		{
			std::cout << SIMULATION_VERSION_INFO << std::endl;
			return 0;
		}

		// Initializing terminal
		if (!SetupSignalHandler())
			return 1;

		// Resolving options
		if (options["seed"])
			RandomSeed = options["seed"];

		auto simulation_duration = options["time"](DEFAULT_SIMULATION_DURATION);
		auto op1_count           = options["op1" ](DEFAULT_OP1_COUNT);
		auto op2_count           = options["op2" ](DEFAULT_OP2_COUNT);
	
		bool shuffle = options["shuffle"];
		auto delay = std::chrono::milliseconds(options["delay"](DEFAULT_DELAY_MS));

		auto max_avg_transaction_queued = options["max-avg-tr"](DEFAULT_AVG_TRANSACTION_QUEUE);
		auto max_queue_size             = options["max-queue" ](DEFAULT_MAX_QUEUE_SIZE       );

		// Optimization mode
		if (options["mode"].as<std::string_view>("interactive") == "optimize")
		{
			Terminal terminal(false);
			TerminalStream stream(&terminal);

			for (size_t test = 1; !WaitSignal(delay); test++)
			{
				Simulation simulation;
				simulation.populate(op1_count, op2_count, shuffle);
				simulation.start();

				while (simulation.getCurrentTime() < simulation_duration)
					simulation.advance();

				stream.clear();
				stream << "Test #" << test << std::endl;
				DisplaySimulationStatus(stream, simulation, DisplayOptions::Optimize);
				stream << std::endl;

				auto op1_avg_load = simulation.getAverageOperatorLoad(Operator::Type::Operator1);
				auto op2_avg_load = simulation.getAverageOperatorLoad(Operator::Type::Operator2);

				if (
					simulation.getMaxQueueSize() <= max_queue_size &&
					simulation.getAverageQueuedTransactions(Transaction::Transaction1) <= max_avg_transaction_queued && 
					simulation.getAverageQueuedTransactions(Transaction::Transaction1) <= max_avg_transaction_queued
				)
				{
					stream.pushForeground(Terminal::Color::Green);
					stream << "Test complete" << std::endl;
					stream.popForeground();
					stream.display();

					break;
				}

				if (op1_avg_load > op2_avg_load)
				{
					stream << "Increasing operators of type 1 number" << std::endl;
					op1_count++;
				}

				else
				{
					stream << "Increasing operators of type 2 number" << std::endl;
					op2_count++;
				}

				stream.pushForeground(Terminal::Color::Yellow);
				stream << "Restarting test" << std::endl;
				stream.popForeground();

				stream.display();
			}
		}

		// Interactive mode
		else
		{          
			Simulation simulation;
			simulation.populate(op1_count, op2_count, shuffle);
			simulation.start();

			Terminal terminal(true);
			TerminalStream stream(&terminal);

			while (!WaitSignal(delay))
			{
				stream.clear();

				DisplaySimulationStatus(stream, simulation);
				simulation.advance();

				stream.display();
			}
		}
	}

	catch (const std::exception& exc)
	{
		std::cerr << exc.what() << std::endl;
		return 1;
	}

	return 0;
}

//======================================

void DisplaySimulationStatus(
	TerminalStream& stream,
	Simulation& simulation,
	DisplayOptions options /*= DisplayOptions::Default*/
)
{
	size_t padding = stream.getPadding();

	stream << "Time: " << simulation.getCurrentTime() << std::endl;
	stream << "Seed: " << RandomSeed << std::endl << std::endl;

	// Transaction queue
	if (options & DisplayOptions::TransactionQueue)
	{
		stream << "Transaction queue: ";

		auto& transaction_queue = simulation.getTransactionQueue();

		if (!transaction_queue.empty())
		{
			size_t max_transaction_idx = std::min<size_t>(
				DISPLAY_MAX_TRANSACTION_QUEUE, 
				transaction_queue.size()
			);

			for (size_t i = 0; i < max_transaction_idx; i++)
				stream << transaction_queue[i] << ' ';

			if (transaction_queue.size() > DISPLAY_MAX_TRANSACTION_QUEUE)
				stream << "... +" << transaction_queue.size() - DISPLAY_MAX_TRANSACTION_QUEUE;

			stream << std::endl;
		}

		else
			stream << "Empty" << std::endl;

		stream << std::endl;
	}

	size_t y = stream.getPosition().second;

	// Operators
	if (options & DisplayOptions::Operators)
	{
		stream << "Operators:" << std::endl;

		auto& operators = simulation.getOperators();
		if (options & DisplayOptions::OperatorsDetails)
		{
			for (const auto& op: operators)
			{
				stream << *op << " - ";
				op->printStatus(stream);
				stream << std::endl;
			}
		}

		else
		{
			for (size_t i = 0; i < static_cast<size_t>(Operator::Type::Amount); i++)
			{
				auto count = std::count_if(
					operators.begin(),
					operators.end(),
					[i](const Operator* op) -> bool
					{
						return op->getType() == static_cast<Operator::Type>(i);
					}
				);

				stream << count << " operators of type " << i + 1 << std::endl;
			}
		}

		stream << std::endl;
	}

	// Statistics
	if (options & DisplayOptions::Statistics)
	{
		stream << "Statistics:" << std::endl;
		stream << "Max queue size: " << simulation.getMaxQueueSize() << std::endl;

		for (size_t i = 0; i < static_cast<size_t>(Transaction::Amount); i++)
		{
			auto transaction = static_cast<Transaction>(i);

			stream << "Avrage transactions ";
			stream << transaction << " queued: ";

			if (simulation.getCurrentTime() > 0)
				stream 
					<< std::setprecision(3) 
				<< simulation.getAverageQueuedTransactions(transaction) << std::endl;

			else
				stream << "N/A" << std::endl;
		}

		for (size_t i = 0; i < static_cast<size_t>(Operator::Type::Amount); i++)
		{
			stream << "Operator of type " << 1 + i << " average load: ";

			if (simulation.getCurrentTime() > 0)
				stream 
					<< std::setprecision(3) 
					<< simulation.getAverageOperatorLoad(static_cast<Operator::Type>(i)) << std::endl;

			else
				stream << "N/A" << std::endl;
		}
	}

	// Event queue
	if (options & DisplayOptions::EventQueue)
	{
		stream.setPosition(0, y);
		stream.setPadding(45);

		stream << "Upcoming events:" << std::endl;

		auto& event_queue = simulation.getEventQueue();
		if (!event_queue.empty())
		{
			size_t last_event_idx = std::min<size_t>(
				DISPLAY_MAX_EVENT_QUEUE,
				event_queue.size()
			);

			auto end = std::next(event_queue.begin(), last_event_idx);
			size_t time_digits = 1 + std::log10((*std::prev(end))->getTime());

			for (auto iter = event_queue.begin(); iter != end; ++iter)
			{
				auto& event = *iter;

				stream << std::setw(time_digits) << event->getTime() << ": ";
				stream << *event << std::endl;
			}

			if (event_queue.size() > DISPLAY_MAX_EVENT_QUEUE)
				stream << "... +" << event_queue.size() - DISPLAY_MAX_EVENT_QUEUE;
		}
	}

	stream.setPadding(padding);
}

//======================================