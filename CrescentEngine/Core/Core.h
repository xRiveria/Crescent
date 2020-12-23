#pragma once
#include <exception>
#include <windows.h>   // WinApi header

//To Do: Colors for log messages, Conditions.

namespace CrescentEngine
{
	static inline HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	static inline int currentColorID = 0;

	static inline void ChangeConsoleTextColor(int colorID)
	{
		if (colorID != currentColorID)
		{
			currentColorID = colorID;
		}
		SetConsoleTextAttribute(hConsole, currentColorID);
	}

#define CrescentInfo(x)	   ChangeConsoleTextColor(2); std::cout << "[INFO] " << x << "\n";	
#define CrescentError(x)   ChangeConsoleTextColor(4); std::cout << "[ERROR] " << x << "\n"; std::terminate();
}
