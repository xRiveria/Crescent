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

#define CrescentInfo(x)	   ChangeConsoleTextColor(2); std::cout << "[INFO] " << x << "\n"; ChangeConsoleTextColor(15);	
#define CrescentWarn(x)    ChangeConsoleTextColor(14); std::cout << "[WARN] " << x << "\n"; ChangeConsoleTextColor(15);	
#define CrescentError(x)   ChangeConsoleTextColor(4); std::cout << "[ERROR] " << x << "\n"; ChangeConsoleTextColor(15); std::terminate();
}
