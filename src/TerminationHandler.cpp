#include <iostream>

#include "TerminationHandler.hpp"

//======================================

std::atomic_bool TerminationReceived = false;

//======================================

#ifdef SIMULATION_WINDOWS

#include <Windows.h>

BOOL WINAPI ConsoleCtrlHandlerRoutine(DWORD signal)
{
	TerminationReceived = true;
	return TRUE;
}

void SetupTerminationHandler()
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
	}
}

#elif

#endif

//======================================