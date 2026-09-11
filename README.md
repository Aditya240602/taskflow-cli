# Taskflow CLI

A color-coded, persistent task manager for the terminal, written in modern C++17.

```
 _____         _    __ _               ____ _     ___
|_   _|_ _ ___| | _/ _| | _____      __/ ___| |   |_ _|
  | |/ _` / __| |/ / |_| |/ _ \ \ /\ / / |   | |    | |
  | | (_| \__ \   <|  _| | (_) \ V  V /| |___| |___ | |
  |_|\__,_|___/_|\_\_| |_|\___/ \_/\_/  \____|_____|___|
```

## Features

- **Full CRUD** — add, edit, complete, and delete tasks
- **Priorities** — High / Medium / Low, color-coded in the terminal
- **Due dates** — optional, used for sorting and display
- **Search** — case-insensitive keyword search across title and description
- **Sort** — by priority, due date, or ID
- **Filter** — view only High / Medium / Low priority tasks
- **Persistence** — tasks are saved to `tasks.json` after every change and reloaded automatically on startup
- **CSV export** — for spreadsheets or reporting
- **Zero external dependencies** — the JSON persistence layer is hand-written for this project's exact schema, so the whole thing builds with nothing but a C++17 compiler

## Project structure

```
taskflow-cli/
├── main.cpp              Menu-driven CLI entry point
├── Task.h                Task struct and Priority enum
├── TaskManager.h/.cpp    Core logic: CRUD, search, sort, filter, CSV export
├── Display.h/.cpp        ANSI colors, headers, formatted task table
├── JsonStore.h           Minimal JSON read/write for the Task schema
├── tests/
│   └── test_task_manager.cpp   Dependency-free test suite (30+ assertions)
└── CMakeLists.txt
```

## Building

### With CMake (recommended)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

This produces two binaries in `build/`:
- `taskflow` — the CLI itself
- `taskflow_tests` — the test suite

### With g++ directly

```bash
g++ -std=c++17 -Wall -Wextra -O2 -o taskflow main.cpp TaskManager.cpp Display.cpp
```

## Running

```bash
./build/taskflow
```

Tasks are saved to `tasks.json` in the current directory and reloaded automatically the next time you run the program.

## Running the tests

```bash
cd build
ctest --output-on-failure
```

or run the test binary directly for verbose PASS/FAIL output:

```bash
./build/taskflow_tests
```

The suite covers add/edit/complete/delete, filtering, search, sorting, CSV
export, persistence across separate `TaskManager` instances, and graceful
handling of a missing save file.

## Design notes

- **No external JSON library.** `JsonStore.h` is a small, purpose-built
  reader/writer for this project's flat task schema rather than a
  general-purpose parser — a deliberate tradeoff to keep the build
  dependency-free.
- **`TaskManager` is UI-agnostic.** It only touches data and files; `main.cpp`
  owns all I/O and prompting, and `Display` owns all formatting. This split
  is also what lets the test binary link against the same logic the CLI uses
  without pulling in any terminal interaction.
- **Autosave.** Every mutating operation (`addTask`, `editTask`,
  `completeTask`, `deleteTask`) writes to disk immediately, so there's no
  separate "save" step to forget.

## Possible extensions

- Tags/labels and filtering by tag
- Recurring tasks
- Interactive TUI (e.g. with ncurses/FTXUI) instead of a menu loop
- Multiple task lists / projects
