#include <iostream>
#include <chrono>
#include <string_view>
#include <cstring>

#include "Distribution.hpp"
#include "Config.hpp"
#include "SignalHandler.hpp"
#include "Simulation.hpp"
#include "EscapeSequence.hpp"
#include "ArgParser.hpp"
#include "SignalHandler.hpp"

using namespace std::chrono_literals;

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
			{"shuffle",    'S', "Enable operator shuffling"                                  },
			{"pause",           "Prompt interactive mode to continue after each event"       }
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
		std::cout << TermColor::Push(TermColor::ForegroundDefault);
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
			for (size_t test = 1; !WaitSignal(delay); test++)
			{
				Simulation simulation(
					op1_count, 
					op2_count, 
					max_avg_transaction_queued, 
					max_queue_size, 
					shuffle
				);

				while (simulation.getCurrentTime() < simulation_duration)
					simulation.advance();

				std::cout 
					<< EscapeSequence::Clear << TermCursorPos(0, 0) 
					<< "Test #" << test << ":" << std::endl;

				simulation.displayStatistics(
					std::cout, 
					Simulation::Default 
						& ~Simulation::OperatorsDetails
						& ~Simulation::Queue
						& ~Simulation::EventQueue
				);

				std::cout << std::endl;

				auto op1_avg_load = simulation.getAverageOperatorLoad<Operator1>();
				auto op2_avg_load = simulation.getAverageOperatorLoad<Operator2>();

				std::cout 
					<< "Operator1 average load: " 
					<< std::setprecision(2) << std::fixed << op1_avg_load << std::endl;

				std::cout 
					<< "Operator2 average load: " 
					<< std::setprecision(2) << std::fixed << op2_avg_load << std::endl;

				std::cout << std::endl;

				if (simulation)
				{
					std::cout 
						<< TermColor::Push(TermColor::ForegroundGreen)
						<< "Test complete" 
						<< TermColor::Pop()
						<< std::endl;

					break;
				}

				if (op1_avg_load > op2_avg_load)
				{
					std::cout << "Increasing operators of type 1 number" << std::endl;
					op1_count++;
				}

				else
				{
					std::cout << "Increasing operators of type 2 number" << std::endl;
					op2_count++;
				}

				std::cout 
					<< TermColor::Push(TermColor::ForegroundYellow)
					<< "Restarting test"
					<< TermColor::Pop()
					<< std::endl;

			}
		}

		// Interactive mode
		else
		{                           
			std::cout << EscapeSequence::EnableAlternativeBuffer;

			Simulation simulation(
				op1_count, 
				op2_count, 
				max_avg_transaction_queued, 
				max_queue_size, 
				shuffle
			);

			while (true)
			{
				std::cout << EscapeSequence::Clear << TermCursorPos(0, 0);
				simulation.displayStatistics(std::cout, Simulation::Default & ~Simulation::Statistics);

				if (options["pause"])
				{
					std::cin.get();
					if (WaitSignal(0ms))
						break;
				}

				else if (WaitSignal(delay))
					break;

				simulation.advance();
			}

			std::cout << EscapeSequence::DisableAlternativeBuffer;
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
