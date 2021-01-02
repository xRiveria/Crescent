#pragma once
#include <exception>
#include <windows.h>   // WinApi header

//To Do: Colors for log messages, Conditions.

namespace Crescent
{
	static inline HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	static inline int currentColorID = 0;

	static inline void ChangeConsoleTextColor(int colorID)
	{
		if (colorID != currentColorID)
		{
			currentColorID = colorID;
		}
		//Expensive, but managable for now.
		SetConsoleTextAttribute(hConsole, currentColorID);
	}

#define CrescentInfo(x)	   Crescent::ChangeConsoleTextColor(2); std::cout << "[INFO] " << x << "\n";  Crescent::ChangeConsoleTextColor(15);	
#define CrescentWarn(x)    Crescent::ChangeConsoleTextColor(14); std::cout << "[WARN] " << x << "\n"; Crescent::ChangeConsoleTextColor(15);	
#define CrescentError(x)   Crescent::ChangeConsoleTextColor(4); std::cout << "[ERROR] " << x << "\n"; Crescent::ChangeConsoleTextColor(15); std::terminate();
}
