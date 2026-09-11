#include "TaskManager.h"
#include "JsonStore.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>

TaskManager::TaskManager(std::string storagePath)
    : storagePath_(std::move(storagePath)), nextId_(1) {
    load();
}

void TaskManager::load() {
    try {
        tasks_ = JsonStore::load(storagePath_);
        nextId_ = computeNextId();
    } catch (const std::exception&) {
        // Corrupt or unreadable file: start fresh rather than crash.
        tasks_.clear();
        nextId_ = 1;
    }
}

void TaskManager::save() const {
    JsonStore::save(storagePath_, tasks_);
}

int TaskManager::computeNextId() const {
    int maxId = 0;
    for (const auto& t : tasks_) maxId = std::max(maxId, t.id);
    return maxId + 1;
}

std::vector<Task>::iterator TaskManager::findById(int id) {
    return std::find_if(tasks_.begin(), tasks_.end(),
                         [id](const Task& t) { return t.id == id; });
}

void TaskManager::addTask(const std::string& title, const std::string& description,
                           Priority priority, const std::string& dueDate) {
    Task t;
    t.id = nextId_++;
    t.title = title;
    t.description = description;
    t.priority = priority;
    t.dueDate = dueDate;
    t.completed = false;
    tasks_.push_back(t);
    save();
}

bool TaskManager::editTask(int id, const std::string& title, const std::string& description,
                            Priority priority, const std::string& dueDate) {
    auto it = findById(id);
    if (it == tasks_.end()) return false;
    it->title = title;
    it->description = description;
    it->priority = priority;
    it->dueDate = dueDate;
    save();
    return true;
}

bool TaskManager::completeTask(int id) {
    auto it = findById(id);
    if (it == tasks_.end()) return false;
    it->completed = true;
    save();
    return true;
}

bool TaskManager::deleteTask(int id) {
    auto it = findById(id);
    if (it == tasks_.end()) return false;
    tasks_.erase(it);
    save();
    return true;
}

bool TaskManager::isEmpty() const {
    return tasks_.empty();
}

std::vector<Task> TaskManager::all() const {
    return tasks_;
}

std::vector<Task> TaskManager::filterByPriority(Priority p) const {
    std::vector<Task> out;
    std::copy_if(tasks_.begin(), tasks_.end(), std::back_inserter(out),
                 [p](const Task& t) { return t.priority == p; });
    return out;
}

std::vector<Task> TaskManager::search(const std::string& keyword) const {
    std::vector<Task> out;
    if (keyword.empty()) return out;

    std::string needle = keyword;
    std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);

    for (const auto& t : tasks_) {
        std::string title = t.title, desc = t.description;
        std::transform(title.begin(), title.end(), title.begin(), ::tolower);
        std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);
        if (title.find(needle) != std::string::npos ||
            desc.find(needle) != std::string::npos) {
            out.push_back(t);
        }
    }
    return out;
}

std::vector<Task> TaskManager::sortedBy(SortKey key) const {
    std::vector<Task> out = tasks_;
    switch (key) {
        case SortKey::PRIORITY:
            std::stable_sort(out.begin(), out.end(), [](const Task& a, const Task& b) {
                return static_cast<int>(a.priority) < static_cast<int>(b.priority);
            });
            break;
        case SortKey::DUE_DATE:
            std::stable_sort(out.begin(), out.end(), [](const Task& a, const Task& b) {
                // Tasks without a due date sort last.
                if (a.dueDate.empty()) return false;
                if (b.dueDate.empty()) return true;
                return a.dueDate < b.dueDate;
            });
            break;
        case SortKey::ID:
            std::stable_sort(out.begin(), out.end(), [](const Task& a, const Task& b) {
                return a.id < b.id;
            });
            break;
    }
    return out;
}

bool TaskManager::exportToCsv(const std::string& path) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out) return false;

    auto csvEscape = [](const std::string& s) {
        bool needsQuotes = s.find(',') != std::string::npos ||
                            s.find('"') != std::string::npos ||
                            s.find('\n') != std::string::npos;
        if (!needsQuotes) return s;
        std::string escaped = "\"";
        for (char c : s) {
            if (c == '"') escaped += "\"\"";
            else escaped += c;
        }
        escaped += "\"";
        return escaped;
    };

    out << "id,title,description,priority,dueDate,completed\n";
    for (const auto& t : tasks_) {
        out << t.id << ","
            << csvEscape(t.title) << ","
            << csvEscape(t.description) << ","
            << priorityToString(t.priority) << ","
            << csvEscape(t.dueDate) << ","
            << (t.completed ? "true" : "false") << "\n";
    }
    return true;
}
