#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <string_view>
#include <cstring>

#include "Distribution.hpp"
#include "Config.hpp"
#include "Simulation.hpp"
#include "EscapeSequence.hpp"
#include "ArgParser.hpp"

using namespace std::chrono_literals;

//======================================

int main(int argc, char* argv[])
{
	ArgParser parser {
		{"help",            "Print usage reference and exit"                       },
		{"version",         "Print build information and exit"                     },
		{"seed",            "Set random seed"                                      },
		{"mode",            "Set simulation mode"                                  },
		{"time",            "Set simulation duration"                              },
		{"delay",           "Set delay in milliseconds"                            },
		{"max-avg-tr", 'X', "Set target average transactions limit (optimize mode)"},
		{"max-queue",  'Y', "Set target queue size limit (optimize mode)"          },
		{"op1",        '1', "Set number of operators of type 1"                    },
		{"op2",        '2', "Set number of operators of type 2"                    },
		{"shuffle",    'S', "Enable operator shuffling"                            }
	};

	parser.parse(argc, argv);

	// Display help and exit
	if (parser["help"])
	{
		std::cout << parser << std::endl;
		return 0;
	}

	// Display version and exit
	if (parser["version"])
	{
		std::cout << SIMULATION_VERSION_INFO << std::endl;
		return 0;
	}

	std::cout << TermColor::Push(TermColor::ForegroundDefault);

	if (parser["seed"])
		RandomSeed = parser["seed"];

	// Resolving options
	auto simulation_duration = parser.get("time", DEFAULT_SIMULATION_DURATION);
	auto op1_count           = parser.get("op1",  DEFAULT_OP1_COUNT          );
	auto op2_count           = parser.get("op2",  DEFAULT_OP2_COUNT          );
	
	bool shuffle = parser["shuffle"];
	auto delay = std::chrono::milliseconds(parser.get("delay", DEFAULT_DELAY_MS));

	auto max_avg_transaction_queued = parser.get("max-avg-tr", DEFAULT_AVG_TRANSACTION_QUEUE);
	auto max_queue_size             = parser.get("max-queue", DEFAULT_MAX_QUEUE_SIZE        );

	// Optimization mode
	if (parser.get("mode", "optimize") == "optimize")
	{
		for (size_t test = 1; true; test++)
		{
			Simulation simulation(
				op1_count, 
				op2_count, 
				max_avg_transaction_queued, 
				max_queue_size, 
				shuffle
			);

			for (size_t time = 0; time < simulation_duration; time++)
				simulation.onTimeTick();

			std::cout 
				<< TermClear() << TermCursorPos(0, 0) 
				<< "Test #" << test << ":" << std::endl;

			simulation.displayStatistics(
				std::cout, 
				Simulation::Default 
					& ~Simulation::OperatorsDetails
					& ~Simulation::Queue
					& ~Simulation::PendingTransactions
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

			std::this_thread::sleep_for(delay);
		}
	}

	// Interactive mode
	else
	{                           
		Simulation simulation(
			op1_count, 
			op2_count, 
			max_avg_transaction_queued, 
			max_queue_size, 
			shuffle
		);

		for (size_t time = 0; time < simulation_duration; time++)
		{
			simulation.onTimeTick();

			std::cout << TermClear() << TermCursorPos(0, 0);
			simulation.displayStatistics();

			std::this_thread::sleep_for(delay);
		}
	}
}

//======================================