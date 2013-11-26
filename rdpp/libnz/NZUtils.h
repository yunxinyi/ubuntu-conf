#pragma once

#ifdef NDEBUG
#define NZAssert(cond) ((void)0)
#else

#define NZAssert(cond) \
	((cond) ? (void)0 : NZUtils::assert(#cond, __FILE__, __LINE__))
#endif

#include <iostream>
namespace NZUtils {
	inline void assert(const char* cond, const char* file_, int line) {
		std::cout << "assert(" << cond << ") failed, file: " << file_
			<< ", line: " << line << std::endl;
		exit(1);
	}
	inline void printProgress(size_t percent, const char* hint = "") {
		if (!hint || strlen(hint) > 20)
			hint = "Progress:";
		char buf[51] = "--------------------------------------------------";
		/*
		if (percent%2)
			buf[(percent+1)>>1] = '>';
		*/
		for (size_t i = 0; i < percent; ++i) {
			buf[i/2] = '=';
		}
		std::cout << '\r' << hint << ": [" << buf << "] " << percent << "%";
		if (percent == 100)
			std::cout << std::endl;
	}
	inline bool NZConfirm(const std::string& prompt) {
		std::cout << prompt << "? [Y/N]:";
		char c;
		while (std::cin >> c) {
			if (c == 'Y' || c == 'y')
				return true;
			else if (c == 'N' || c == 'n')
				return false;
			else {
				std::cout << "[Y/N]: ";
			}
		}
		return false;
	}
}
