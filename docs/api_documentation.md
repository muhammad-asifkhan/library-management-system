# API / Developer Notes (Quick)

This project is a C++17 desktop application (not a web API). The key public-facing interfaces are the core classes and the GUI entry point.

## Main entry point

- `src/main_gui.cpp`: starts the GUI (`MainWindow`) and runs the event loop.

## Core orchestrator

- `include/core/LibrarySystem.hpp`: main integration class (singleton) that exposes:
  - Book CRUD and search
  - Student registration/search
  - Issue/Return workflow
  - Request queues
  - Transaction history and statistics
  - Save/load (CSV)

## GUI

- `include/gui/MainWindow.hpp` / `src/gui/MainWindow.cpp`:
  - Login gate (Student/Librarian)
  - Role-based permissions (Librarian vs Student)
  - Student Assistant (offline recommender)

## Data files

Stored in `data/` as CSV:
- `books.csv`
- `students.csv`
- `transactions.csv`
- `requests.csv`

Full documentation: `docs/latex/main.pdf`.

