#pragma once
#include <exception>

//To Do: Colors for log messages, Conditions.

namespace CrescentEngine
{
#define CrescentInfo(x)	   std::cout << "[INFO] " << x << "\n";	
#define CrescentError(x)   std::cout << "[ERROR] " << x << "\n"; std::terminate();
}
