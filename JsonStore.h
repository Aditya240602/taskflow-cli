#pragma once
// Minimal, dependency-free JSON reader/writer for a vector<Task>.
//
// This is NOT a general-purpose JSON library — it only understands the
// exact flat schema Taskflow-CLI writes for itself. That's a deliberate
// tradeoff: no external dependency to install, at the cost of generality.

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "Task.h"

namespace JsonStore {

inline std::string escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;
        }
    }
    return out;
}

inline std::string unescape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[i + 1];
            switch (n) {
                case '"':  out += '"';  ++i; break;
                case '\\': out += '\\'; ++i; break;
                case 'n':  out += '\n'; ++i; break;
                case 'r':  out += '\r'; ++i; break;
                case 't':  out += '\t'; ++i; break;
                default:   out += s[i];
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

// Writes tasks to a JSON array of flat objects.
inline void save(const std::string& path, const std::vector<Task>& tasks) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) throw std::runtime_error("Could not open '" + path + "' for writing.");

    out << "[\n";
    for (size_t i = 0; i < tasks.size(); ++i) {
        const Task& t = tasks[i];
        out << "  {\n";
        out << "    \"id\": " << t.id << ",\n";
        out << "    \"title\": \"" << escape(t.title) << "\",\n";
        out << "    \"description\": \"" << escape(t.description) << "\",\n";
        out << "    \"priority\": \"" << priorityToString(t.priority) << "\",\n";
        out << "    \"dueDate\": \"" << escape(t.dueDate) << "\",\n";
        out << "    \"completed\": " << (t.completed ? "true" : "false") << "\n";
        out << "  }" << (i + 1 < tasks.size() ? "," : "") << "\n";
    }
    out << "]\n";
}

// Parses the flat schema written by save(). Tolerant of whitespace/newline
// formatting differences, but assumes the same field set.
inline std::vector<Task> load(const std::string& path) {
    std::vector<Task> tasks;
    std::ifstream in(path);
    if (!in) return tasks; // no file yet == empty task list, not an error

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string content = buffer.str();

    size_t pos = 0;
    auto findObject = [&](size_t from) -> std::pair<size_t, size_t> {
        size_t start = content.find('{', from);
        if (start == std::string::npos) return {std::string::npos, std::string::npos};
        size_t end = content.find('}', start);
        if (end == std::string::npos) return {std::string::npos, std::string::npos};
        return {start, end};
    };

    auto extractString = [&](const std::string& obj, const std::string& key) -> std::string {
        std::string needle = "\"" + key + "\"";
        size_t kpos = obj.find(needle);
        if (kpos == std::string::npos) return "";
        size_t colon = obj.find(':', kpos);
        size_t firstQuote = obj.find('"', colon + 1);
        if (firstQuote == std::string::npos) return "";
        size_t secondQuote = firstQuote + 1;
        std::string val;
        while (secondQuote < obj.size()) {
            if (obj[secondQuote] == '\\') { val += obj[secondQuote]; val += obj[secondQuote + 1]; secondQuote += 2; continue; }
            if (obj[secondQuote] == '"') break;
            val += obj[secondQuote];
            ++secondQuote;
        }
        return unescape(val);
    };

    auto extractInt = [&](const std::string& obj, const std::string& key) -> int {
        std::string needle = "\"" + key + "\"";
        size_t kpos = obj.find(needle);
        if (kpos == std::string::npos) return 0;
        size_t colon = obj.find(':', kpos);
        size_t end = colon + 1;
        while (end < obj.size() && (obj[end] == ' ' || obj[end] == '\n')) ++end;
        size_t stop = end;
        while (stop < obj.size() && (isdigit((unsigned char)obj[stop]) || obj[stop] == '-')) ++stop;
        if (stop == end) return 0;
        return std::stoi(obj.substr(end, stop - end));
    };

    auto extractBool = [&](const std::string& obj, const std::string& key) -> bool {
        std::string needle = "\"" + key + "\"";
        size_t kpos = obj.find(needle);
        if (kpos == std::string::npos) return false;
        size_t colon = obj.find(':', kpos);
        return obj.find("true", colon) < obj.find("false", colon);
    };

    while (true) {
        std::pair<size_t, size_t> range = findObject(pos);
        size_t start = range.first;
        size_t end = range.second;
        if (start == std::string::npos) break;
        std::string obj = content.substr(start, end - start + 1);

        Task t;
        t.id = extractInt(obj, "id");
        t.title = extractString(obj, "title");
        t.description = extractString(obj, "description");
        t.priority = priorityFromString(extractString(obj, "priority"));
        t.dueDate = extractString(obj, "dueDate");
        t.completed = extractBool(obj, "completed");
        tasks.push_back(t);

        pos = end + 1;
    }

    return tasks;
}

} // namespace JsonStore
