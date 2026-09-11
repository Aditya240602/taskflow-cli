#include "Display.h"
#include <iostream>
#include <iomanip>

void printDivider(const std::string& ch, int width) {
    std::cout << Color::DIM;
    for (int i = 0; i < width; ++i) std::cout << ch;
    std::cout << Color::RESET << "\n";
}

void printHeader(const std::string& text) {
    std::cout << "\n" << Color::BOLD << Color::BWHITE << " " << text << "\n" << Color::RESET;
    printDivider("-", static_cast<int>(text.size()) + 2);
}

void printSuccess(const std::string& text) {
    std::cout << Color::BGREEN << " [OK] " << Color::RESET << text << "\n";
}

void printError(const std::string& text) {
    std::cout << Color::BRED << " [ERROR] " << Color::RESET << text << "\n";
}

void printInfo(const std::string& text) {
    std::cout << Color::BCYAN << " [i] " << Color::RESET << text << "\n";
}

static const char* priorityColor(Priority p) {
    switch (p) {
        case Priority::HIGH:   return Color::BRED;
        case Priority::MEDIUM: return Color::BYELLOW;
        case Priority::LOW:    return Color::BGREEN;
    }
    return Color::RESET;
}

void printTaskTable(const std::vector<Task>& tasks) {
    if (tasks.empty()) {
        printInfo("No tasks to show.");
        return;
    }

    std::cout << Color::BOLD
               << std::left << std::setw(4)  << "ID"
               << std::setw(24) << "Title"
               << std::setw(9)  << "Priority"
               << std::setw(12) << "Due"
               << std::setw(10) << "Status"
               << Color::RESET << "\n";
    printDivider("-", 58);

    for (const auto& t : tasks) {
        std::string title = t.title.size() > 22 ? t.title.substr(0, 19) + "..." : t.title;
        std::string due = t.dueDate.empty() ? "-" : t.dueDate;

        std::cout << std::left << std::setw(4) << t.id
                   << std::setw(24) << title
                   << priorityColor(t.priority) << std::setw(9) << priorityToString(t.priority) << Color::RESET
                   << std::setw(12) << due
                   << (t.completed ? Color::GREEN : Color::DIM)
                   << std::setw(10) << (t.completed ? "Done" : "Pending")
                   << Color::RESET << "\n";
    }
}
