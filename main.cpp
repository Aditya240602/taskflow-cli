#include <iostream>
#include <limits>
#include <string>
#include "TaskManager.h"
#include "Display.h"

#ifdef _WIN32
#include <windows.h>
void enableAnsi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleOutputCP(CP_UTF8);
}
#else
void enableAnsi() {}
#endif

// ---- Input helpers ---------------------------------------------------

static int getInt(const std::string& prompt) {
    int val;
    while (true) {
        std::cout << Color::BYELLOW << " >> " << prompt << Color::RESET;
        if (std::cin >> val) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return val;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        printError("Please enter a valid number.");
    }
}

static std::string getString(const std::string& prompt) {
    std::string val;
    std::cout << Color::BYELLOW << " >> " << prompt << Color::RESET;
    std::getline(std::cin, val);
    return val;
}

static Priority getPriority() {
    while (true) {
        std::cout << Color::BYELLOW << " >> Priority [1] High [2] Medium [3] Low : " << Color::RESET;
        int c;
        if (std::cin >> c) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (c == 1) return Priority::HIGH;
            if (c == 2) return Priority::MEDIUM;
            if (c == 3) return Priority::LOW;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        printError("Enter 1, 2, or 3.");
    }
}

// ---- Screens ------------------------------------------------------------

static void splash() {
    std::cout << "\n" << Color::BOLD << Color::BRED;
    std::cout << " _____         _    __ _               ____ _     ___ \n";
    std::cout << "|_   _|_ _ ___| | _/ _| | _____      __/ ___| |   |_ _|\n";
    std::cout << "  | |/ _` / __| |/ / |_| |/ _ \\ \\ /\\ / / |   | |    | | \n";
    std::cout << "  | | (_| \\__ \\   <|  _| | (_) \\ V  V /| |___| |___ | | \n";
    std::cout << "  |_|\\__,_|___/_|\\_\\_| |_|\\___/ \\_/\\_/  \\____|_____|___|\n";
    std::cout << Color::RESET << Color::DIM;
    std::cout << " ------------------------------------------------------\n";
    std::cout << "  CLI Task Manager -- v1.1 (with persistence)\n";
    std::cout << " ------------------------------------------------------\n";
    std::cout << Color::RESET << "\n";
}

static void showMenu() {
    std::cout << "\n";
    printDivider("-", 44);
    std::cout << Color::BOLD << Color::BWHITE << " MAIN MENU\n" << Color::RESET;
    printDivider("-", 44);
    std::cout << Color::CYAN   << " [1]" << Color::RESET << " View all tasks\n";
    std::cout << Color::CYAN   << " [2]" << Color::RESET << " Filter by priority\n";
    std::cout << Color::CYAN   << " [3]" << Color::RESET << " Search tasks\n";
    std::cout << Color::CYAN   << " [4]" << Color::RESET << " Sort tasks\n";
    std::cout << Color::BGREEN << " [5]" << Color::RESET << " Add task\n";
    std::cout << Color::YELLOW << " [6]" << Color::RESET << " Edit task\n";
    std::cout << Color::BGREEN << " [7]" << Color::RESET << " Mark task as complete\n";
    std::cout << Color::BRED   << " [8]" << Color::RESET << " Delete task\n";
    std::cout << Color::CYAN   << " [9]" << Color::RESET << " Export to CSV\n";
    std::cout << Color::DIM    << " [0]" << Color::RESET << " Exit\n";
    printDivider("-", 44);
}

// ---- Actions --------------------------------------------------------------

static void doAdd(TaskManager& mgr) {
    printHeader("ADD TASK");
    std::string title = getString("Title : ");
    if (title.empty()) { printError("Title cannot be empty."); return; }
    std::string desc = getString("Description : ");
    Priority p = getPriority();
    std::string due = getString("Due date (YYYY-MM-DD, blank for none) : ");
    mgr.addTask(title, desc, p, due);
    printSuccess("Task added and saved.");
}

static void doEdit(TaskManager& mgr) {
    printHeader("EDIT TASK");
    printTaskTable(mgr.all());
    if (mgr.isEmpty()) return;
    int id = getInt("Enter Task ID to edit : ");
    std::string title = getString("New title : ");
    if (title.empty()) { printError("Title cannot be empty."); return; }
    std::string desc = getString("New description : ");
    Priority p = getPriority();
    std::string due = getString("New due date (YYYY-MM-DD, blank for none) : ");
    if (mgr.editTask(id, title, desc, p, due))
        printSuccess("Task updated and saved.");
    else
        printError("Task ID not found.");
}

static void doComplete(TaskManager& mgr) {
    printHeader("COMPLETE TASK");
    printTaskTable(mgr.all());
    if (mgr.isEmpty()) return;
    int id = getInt("Enter Task ID to mark complete : ");
    if (mgr.completeTask(id))
        printSuccess("Task marked as completed!");
    else
        printError("Task ID not found.");
}

static void doDelete(TaskManager& mgr) {
    printHeader("DELETE TASK");
    printTaskTable(mgr.all());
    if (mgr.isEmpty()) return;
    int id = getInt("Enter Task ID to delete : ");
    std::cout << Color::BRED << " Are you sure? [y/N] : " << Color::RESET;
    char confirm; std::cin >> confirm;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (confirm != 'y' && confirm != 'Y') { printInfo("Cancelled."); return; }
    if (mgr.deleteTask(id))
        printSuccess("Task deleted.");
    else
        printError("Task ID not found.");
}

static void doFilter(TaskManager& mgr) {
    printHeader("FILTER BY PRIORITY");
    std::cout << Color::CYAN << " [1]" << Color::RESET << Color::BRED    << " High\n"   << Color::RESET;
    std::cout << Color::CYAN << " [2]" << Color::RESET << Color::BYELLOW << " Medium\n" << Color::RESET;
    std::cout << Color::CYAN << " [3]" << Color::RESET << Color::BGREEN  << " Low\n"    << Color::RESET;
    int c = getInt("Choose filter : ");
    Priority p;
    if (c == 1) p = Priority::HIGH;
    else if (c == 2) p = Priority::MEDIUM;
    else if (c == 3) p = Priority::LOW;
    else { printError("Invalid choice."); return; }
    printDivider("-");
    printTaskTable(mgr.filterByPriority(p));
}

static void doSearch(TaskManager& mgr) {
    printHeader("SEARCH TASKS");
    std::string keyword = getString("Keyword (title or description) : ");
    auto results = mgr.search(keyword);
    printDivider("-");
    if (results.empty())
        printInfo("No matching tasks.");
    else
        printTaskTable(results);
}

static void doSort(TaskManager& mgr) {
    printHeader("SORT TASKS");
    std::cout << Color::CYAN << " [1]" << Color::RESET << " By priority\n";
    std::cout << Color::CYAN << " [2]" << Color::RESET << " By due date\n";
    std::cout << Color::CYAN << " [3]" << Color::RESET << " By ID\n";
    int c = getInt("Choose sort key : ");
    SortKey key;
    if (c == 1) key = SortKey::PRIORITY;
    else if (c == 2) key = SortKey::DUE_DATE;
    else if (c == 3) key = SortKey::ID;
    else { printError("Invalid choice."); return; }
    printDivider("-");
    printTaskTable(mgr.sortedBy(key));
}

static void doExport(TaskManager& mgr) {
    printHeader("EXPORT TO CSV");
    std::string path = getString("Output file (e.g. tasks.csv) : ");
    if (path.empty()) path = "tasks.csv";
    if (mgr.exportToCsv(path))
        printSuccess("Exported to " + path);
    else
        printError("Could not write to " + path);
}

// ---- Main -------------------------------------------------------------

int main() {
    enableAnsi();
    TaskManager mgr("tasks.json");
    splash();

    while (true) {
        showMenu();
        int choice = getInt("Choose an option : ");
        switch (choice) {
            case 1: printHeader("ALL TASKS"); printTaskTable(mgr.all()); break;
            case 2: doFilter(mgr); break;
            case 3: doSearch(mgr); break;
            case 4: doSort(mgr); break;
            case 5: doAdd(mgr); break;
            case 6: doEdit(mgr); break;
            case 7: doComplete(mgr); break;
            case 8: doDelete(mgr); break;
            case 9: doExport(mgr); break;
            case 0:
                std::cout << "\n" << Color::BCYAN
                          << " Goodbye! Stay productive.\n\n"
                          << Color::RESET;
                return 0;
            default:
                printError("Invalid option. Try again.");
        }
    }
}
