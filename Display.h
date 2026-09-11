#pragma once
#include <string>
#include <vector>
#include "Task.h"

namespace Color {
    constexpr const char* RESET   = "\033[0m";
    constexpr const char* BOLD    = "\033[1m";
    constexpr const char* DIM     = "\033[2m";
    constexpr const char* CYAN    = "\033[36m";
    constexpr const char* YELLOW  = "\033[33m";
    constexpr const char* GREEN   = "\033[32m";
    constexpr const char* RED     = "\033[31m";
    constexpr const char* BRED    = "\033[91m";
    constexpr const char* BGREEN  = "\033[92m";
    constexpr const char* BYELLOW = "\033[93m";
    constexpr const char* BCYAN   = "\033[96m";
    constexpr const char* BWHITE  = "\033[97m";
}

void printDivider(const std::string& ch = "-", int width = 60);
void printHeader(const std::string& text);
void printSuccess(const std::string& text);
void printError(const std::string& text);
void printInfo(const std::string& text);
void printTaskTable(const std::vector<Task>& tasks);
