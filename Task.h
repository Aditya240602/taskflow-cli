#pragma once
#include <string>

enum class Priority { HIGH, MEDIUM, LOW };

inline std::string priorityToString(Priority p) {
    switch (p) {
        case Priority::HIGH:   return "High";
        case Priority::MEDIUM: return "Medium";
        case Priority::LOW:    return "Low";
    }
    return "Unknown";
}

inline Priority priorityFromString(const std::string& s) {
    if (s == "High")   return Priority::HIGH;
    if (s == "Medium") return Priority::MEDIUM;
    return Priority::LOW;
}

struct Task {
    int id;
    std::string title;
    std::string description;
    Priority priority;
    std::string dueDate;   // format: YYYY-MM-DD, empty string = no due date
    bool completed = false;
};
