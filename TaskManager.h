#pragma once
#include <string>
#include <vector>
#include "Task.h"

enum class SortKey { PRIORITY, DUE_DATE, ID };

class TaskManager {
public:
    explicit TaskManager(std::string storagePath = "tasks.json");

    // Persistence
    void load();
    void save() const;

    // CRUD
    void addTask(const std::string& title, const std::string& description,
                 Priority priority, const std::string& dueDate);
    bool editTask(int id, const std::string& title, const std::string& description,
                  Priority priority, const std::string& dueDate);
    bool completeTask(int id);
    bool deleteTask(int id);

    // Queries
    bool isEmpty() const;
    std::vector<Task> all() const;
    std::vector<Task> filterByPriority(Priority p) const;
    std::vector<Task> search(const std::string& keyword) const;
    std::vector<Task> sortedBy(SortKey key) const;

    // Export
    bool exportToCsv(const std::string& path) const;

private:
    std::string storagePath_;
    std::vector<Task> tasks_;
    int nextId_;

    int computeNextId() const;
    std::vector<Task>::iterator findById(int id);
};
