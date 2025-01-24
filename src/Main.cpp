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

	// Resolving options
	auto simulation_duration = GetNumericOption(options, "time", DEFAULT_SIMULATION_DURATION);
	auto op1_count           = GetNumericOption(options, "op1",  DEFAULT_OP1_COUNT          );
	auto op2_count           = GetNumericOption(options, "op2",  DEFAULT_OP2_COUNT          );
	
	bool shuffle = options.contains("shuffle");

	auto delay = std::chrono::milliseconds(
		GetNumericOption(options, "delay", DEFAULT_DELAY_MS)
	);

	auto max_avg_transaction_queued = GetNumericOption(
		options, 
		"max-avg-tr", 
		DEFAULT_AVG_TRANSACTION_QUEUE
	);

	auto max_queue_size = GetNumericOption(
		options, 
		"max-queue",  
		DEFAULT_MAX_QUEUE_SIZE
	);

	// Optimization mode
	if (options["mode"] == "optimize")
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
