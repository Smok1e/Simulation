#pragma once

#include <chrono>

//======================================

// Initialize system-specific signal handler
bool SetupSignalHandler();

// Wait for interruption signal until timeout passes
bool WaitSignal(std::chrono::nanoseconds timeout);

//======================================
