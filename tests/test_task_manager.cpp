// Lightweight, dependency-free test harness for TaskManager.
//
// No Catch2/GoogleTest is used on purpose: this project has zero external
// dependencies, and pulling one in just for tests would break that. Each
// CHECK prints a PASS/FAIL line; the process exits non-zero if anything
// failed, so it plugs straight into `ctest`.

#include <cstdio>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include "../TaskManager.h"

static int g_failures = 0;

#define CHECK(cond) \
    do { \
        if (cond) { \
            std::printf("  [PASS] %s\n", #cond); \
        } else { \
            std::printf("  [FAIL] %s (line %d)\n", #cond, __LINE__); \
            ++g_failures; \
        } \
    } while (0)

static void section(const char* name) {
    std::printf("\n== %s ==\n", name);
}

static void cleanup(const std::string& path) {
    std::remove(path.c_str());
}

int main() {
    const std::string testFile = "test_tasks.json";
    const std::string csvFile = "test_tasks.csv";
    cleanup(testFile);
    cleanup(csvFile);

    section("Add task");
    {
        TaskManager mgr(testFile);
        CHECK(mgr.isEmpty());
        mgr.addTask("Buy milk", "2%, one gallon", Priority::LOW, "2026-09-15");
        mgr.addTask("Finish thesis", "Chapter 4 draft", Priority::HIGH, "2026-09-20");
        CHECK(!mgr.isEmpty());
        CHECK(mgr.all().size() == 2);
        CHECK(mgr.all()[0].id == 1);
        CHECK(mgr.all()[1].id == 2);
    }

    section("Edit task");
    {
        TaskManager mgr(testFile);
        bool ok = mgr.editTask(1, "Buy oat milk", "Barista blend", Priority::MEDIUM, "2026-09-16");
        CHECK(ok);
        CHECK(mgr.all()[0].title == "Buy oat milk");
        CHECK(mgr.all()[0].priority == Priority::MEDIUM);
        bool badId = mgr.editTask(999, "x", "y", Priority::LOW, "");
        CHECK(!badId);
    }

    section("Complete task");
    {
        TaskManager mgr(testFile);
        CHECK(mgr.completeTask(1));
        CHECK(mgr.all()[0].completed == true);
        CHECK(!mgr.completeTask(999));
    }

    section("Filter by priority");
    {
        TaskManager mgr(testFile);
        auto high = mgr.filterByPriority(Priority::HIGH);
        CHECK(high.size() == 1);
        CHECK(high[0].title == "Finish thesis");
    }

    section("Search");
    {
        TaskManager mgr(testFile);
        auto results = mgr.search("thesis");
        CHECK(results.size() == 1);
        auto caseInsensitive = mgr.search("THESIS");
        CHECK(caseInsensitive.size() == 1);
        auto none = mgr.search("nonexistent-keyword");
        CHECK(none.empty());
    }

    section("Sort");
    {
        TaskManager mgr(testFile);
        auto byDue = mgr.sortedBy(SortKey::DUE_DATE);
        CHECK(byDue.front().dueDate <= byDue.back().dueDate);

        auto byPriority = mgr.sortedBy(SortKey::PRIORITY);
        CHECK(byPriority.front().priority == Priority::HIGH);
    }

    section("Persistence across instances");
    {
        TaskManager mgr1(testFile);
        size_t countBefore = mgr1.all().size();
        mgr1.addTask("Temp task", "will persist", Priority::LOW, "");

        TaskManager mgr2(testFile); // fresh instance, same file
        CHECK(mgr2.all().size() == countBefore + 1);
    }

    section("Delete task");
    {
        TaskManager mgr(testFile);
        size_t before = mgr.all().size();
        CHECK(mgr.deleteTask(1));
        CHECK(mgr.all().size() == before - 1);
        CHECK(!mgr.deleteTask(1)); // already gone
    }

    section("CSV export");
    {
        TaskManager mgr(testFile);
        bool ok = mgr.exportToCsv(csvFile);
        CHECK(ok);
        std::ifstream in(csvFile);
        CHECK(in.good());
        std::string firstLine;
        std::getline(in, firstLine);
        CHECK(firstLine == "id,title,description,priority,dueDate,completed");
    }

    section("Loading a missing file starts empty, not crashing");
    {
        cleanup("does_not_exist.json");
        TaskManager mgr("does_not_exist.json");
        CHECK(mgr.isEmpty());
    }

    cleanup(testFile);
    cleanup(csvFile);

    std::printf("\n%s\n", g_failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
    return g_failures == 0 ? 0 : 1;
}
