#include <chrono>
#include <iostream>
#include <ostream>

#include "SignalHandler.hpp"

//======================================

#ifdef SIMULATION_WINDOWS

#include <Windows.h>

BOOL WINAPI ConsoleCtrlHandlerRoutine(DWORD signal)
{
	TerminationReceived = true;
	return TRUE;
}

bool SetupTerminationHandler()
{
	if (!SetConsoleCtrlHandler(ConsoleCtrlHandlerRoutine, TRUE))
	{
		char* message = nullptr;
		FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			GetLastError(),
			NULL,
			reinterpret_cast<char*>(&message),
			0,
			NULL
		);

		std::cerr << "Unable to set console control handler: " << message << std::endl;
		LocalFree(message);

		return false;
	}

	return true;
}

#else                           

#include <cerrno>
#include <cstring>
#include <signal.h>

bool SetupSignalHandler()
{
	// No need to setup custom signal handler on unix
	return true;
}

bool WaitSignal(std::chrono::nanoseconds nanoseconds)
{
	sigset_t set = {};
	sigemptyset(&set);
	sigaddset(&set, SIGINT);

	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(nanoseconds);

    timespec time = {};
	time.tv_sec = seconds.count();
	time.tv_nsec = (nanoseconds - seconds).count();

	return sigtimedwait(&set, nullptr, &time) == SIGINT;
}

#endif

//======================================
