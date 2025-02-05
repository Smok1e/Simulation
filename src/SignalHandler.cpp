#include <chrono>
#include <iostream>
#include <ostream>
#include <mutex>
#include <condition_variable>

#include "SignalHandler.hpp"

//======================================

#ifdef SIMULATION_WINDOWS

#include <Windows.h>

std::condition_variable InterruptCV;
std::atomic_bool Interrupted = false;

BOOL WINAPI ConsoleCtrlHandlerRoutine(DWORD signal)
{
	if (signal != CTRL_C_EVENT && signal != CTRL_BREAK_EVENT)
		return FALSE;

	Interrupted = true;
	InterruptCV.notify_all();

	return TRUE;
}

bool WaitSignal(std::chrono::nanoseconds timeout)
{
	std::mutex mutex;
	std::unique_lock lock(mutex);
	InterruptCV.wait_for(lock, timeout);

	// Avoid spurious wakeups
	return Interrupted;
}

bool SetupSignalHandler()
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
