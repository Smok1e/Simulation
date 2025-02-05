cmake_minimum_required(VERSION 3.21)

project(
	Simulation
	VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
	Simulation
	"src/Distribution.cpp"
	"src/Transaction.cpp"
	"src/Operator.cpp"
	"src/Simulation.cpp"
	"src/Main.cpp"
	"src/EscapeSequence.cpp"
	"src/ArgParser.cpp"
	"src/SignalHandler.cpp"
)

option(
	STL_DISTRIBUTION 
	"Use C++ <random> api instead of C rand() function"
	ON
)

if (STL_DISTRIBUTION)
	add_compile_definitions(SIMULATION_DISTRIBUTION_USE_STL)
endif ()

if (WIN32)
	add_compile_definitions(SIMULATION_WINDOWS)
endif ()

string(TIMESTAMP CURRENT_TIME "%d.%m.%Y %H:%M:%S")

add_compile_definitions(
	SIMULATION_VERSION_INFO="${CMAKE_PROJECT_NAME} version ${CMAKE_PROJECT_VERSION} built with STL_DISTRIBUTION=${STL_DISTRIBUTION} at ${CURRENT_TIME}"
)
