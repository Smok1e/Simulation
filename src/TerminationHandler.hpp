#pragma once

#include <atomic>

//======================================

// Initialize system-specific signal handler
void SetupTerminationHandler();

// This flag can be used to check whether program received termination signal or not
extern std::atomic_bool TerminationReceived;

//======================================