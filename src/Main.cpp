#include <iostream>
#include <thread>
#include <chrono>
#include <map>
#include <string_view>
#include <cstring>
#include <charconv>

#include "Config.hpp"
#include "Simulation.hpp"
#include "EscapeSequence.hpp"

using namespace std::chrono_literals;

//======================================

std::map<std::string_view, std::string_view> ParseArguments(int argc, char* argv[]);

template<typename T>
T GetNumericOption(
	std::map<std::string_view, std::string_view>& options, 
	const char* name, 
	T default_value
);

//======================================

int main(int argc, char* argv[])
{
	auto options = ParseArguments(argc, argv);

	// Display help and exit
	if (options.contains("help"))
	{
		std::cout 
			<< "Usage: " << argv[0] << " [OPTIONS]" << std::endl
			<< "Available options:" << std::endl
			<< "  --help                 - Print usage information and exit       " << std::endl
			<< "  --mode=<mode>          - Set simulation mode (see below)        " << std::endl
			<< "  --time=<time>          - Set simulation duration                " << std::endl
			<< "  --delay=<milliseconds> - Set delay in                           " << std::endl
			<< "  --max-avg-tr=<X>       - Set maximum average transactions of    " << std::endl
			<< "                           every type in queue for optimize mode  " << std::endl
			<< "  --max-queue=<Y>        - Set maximum transaction queue size     " << std::endl
			<< "                           for optimize mode                      " << std::endl
			<< "  --op1=<number>         - Set starting number of type 1 operators" << std::endl
			<< "  --op2=<number>         - Set starting number of type 2 operators" << std::endl
			<< "  --shuffle              - Shuffle operators                      " << std::endl
			<< "                                                                  " << std::endl
			<< "Available modes:                                                  " << std::endl
			<< "  interactive (default)  - Display simulation statistics in       " << std::endl
			<< "                           real time during the simulation        " << std::endl
			<< "  optimize               - Test incrementing operator amounts     " << std::endl
			<< "                           amounts until statistics meets         " << std::endl
			<< "                           specific requirements                  " << std::endl;

		return 0;
	}

	std::cout << TermColor::Push(TermColor::ForegroundDefault);
	srand(time(0));

	// Common options
	auto simulation_duration = GetNumericOption(options, "time", DEFAULT_SIMULATION_DURATION);
	auto op1_count           = GetNumericOption(options, "op1",  DEFAULT_OP1_COUNT          );
	auto op2_count           = GetNumericOption(options, "op2",  DEFAULT_OP2_COUNT          );
	
	bool shuffle = options.contains("shuffle");

	auto delay = std::chrono::milliseconds(
		GetNumericOption(options, "delay", DEFAULT_DELAY_MS)
	);

	// Optimization mode
	if (options["mode"] == "optimize")
	{
		auto max_avg_transaction_queue = GetNumericOption(
			options, 
			"max-avg-tr", 
			DEFAULT_AVG_TRANSACTION_QUEUE
		);

		auto max_queue = GetNumericOption(
			options, 
			"max-queue",  
			DEFAULT_MAX_QUEUE_SIZE
		);

		bool running = true;
		while (running)
		{
			Simulation simulation(op1_count, op2_count, shuffle);
			for (size_t time = 0; time < simulation_duration; time++)
				simulation.onTimeTick();

			std::cout << TermClear() << TermCursorPos(0, 0);
			simulation.displayStatistics(
				std::cout, 
				Simulation::Default 
					& ~Simulation::OperatorsDetails
					& ~Simulation::Queue
					& ~Simulation::PendingTransactions
			);

			running = false;

			auto avg_queue_1 = simulation.getAverageQueuedTransactions(Transaction::Transaction1);
			auto avg_queue_2 = simulation.getAverageQueuedTransactions(Transaction::Transaction2);

			if (avg_queue_1 > max_avg_transaction_queue)
				op1_count += (running = true);

			if (avg_queue_2 > max_avg_transaction_queue)
				op2_count += (running = true);

			if (simulation.getAverageQueuedTransactions(Transaction::Transaction3) > max_avg_transaction_queue && !running)
				op1_count += (running = true);

			if (simulation.getMaxQueueSize() > max_queue && !running)
			{
				if (avg_queue_1 > avg_queue_2)
					op1_count++;

				else
					op2_count++;

				running = true;
			}

			std::this_thread::sleep_for(delay);
		}
	}

	// Interactive mode
	else
	{                           
		Simulation simulation(op1_count, op2_count, shuffle);
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

const char* GetOptionName(const char* str)
{
	return str == strstr(str, "--")
		? str + 2
		: nullptr;
}

std::map<std::string_view, std::string_view> ParseArguments(int argc, char* argv[])
{
	std::map<std::string_view, std::string_view> options;

	for (size_t i = 1; i < argc; i++)
	{
		if (const char* option = GetOptionName(argv[i]))
		{
			if (i + 1 < argc && !GetOptionName(argv[i+1]))
				options[option] = argv[++i];

			else if (const char* value = strchr(argv[i], '='))
				options[std::string_view(option, value - option)] = value + 1;

			else
				options[option] = "";
		}
	}

	return options;
}

template<typename T>
T GetNumericOption(
	std::map<std::string_view, std::string_view>& options, 
	const char* name, 
	T default_value
)
{
	if (!options.contains(name))
		return default_value;

	auto option = options[name];

	T value;
	return std::from_chars(option.data(), option.data() + option.length(), value).ec == std::errc::invalid_argument
		? default_value
		: value;
}

//======================================
