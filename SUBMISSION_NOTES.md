# Submission Notes (For Teacher)

## Project
**Library Management System (C++17)** — GUI-first desktop application using **Dear ImGui + SFML**, with custom data structures (BST/LinkedList/Queue) and CSV persistence.

## How to Build (Linux/macOS)

```bash
cd /home/muhammad-asif-khan/liberary-management-system
chmod +x scripts/create_build.sh
./scripts/create_build.sh
```

## How to Run

```bash
./build-gui/library_gui
```

## Login (Demo Credentials)

- **Librarian (full privileges)**: `admin` / `admin123`
- **Student (limited privileges + AI assistant)**:
  - `100001` / `0001`
  - `100002` / `0002`

PIN rule: **last 4 digits** of the Student ID.

## Role Permissions (Important)

- **Librarian**
  - Book CRUD (add/edit/delete)
  - Student management (register/delete)
  - Issue + return books
  - Save/Reload data
- **Student**
  - Search/view books
  - Request books (queue)
  - Return books (for their own student ID)
  - **Assistant tab** (offline recommendations/help)

## Where is the Documentation?

- **Final LaTeX Report (PDF)**: `docs/latex/main.pdf`
- **LaTeX Source**: `docs/latex/main.tex`
- **Screenshots used in report**: `resources/screenshots/`

## Data Files

Stored in `data/` as CSV:
- `books.csv`
- `students.csv`
- `transactions.csv`
- `requests.csv`

Backups (if enabled/created): `data/backups/`

## GitHub Repository

Published at: `https://github.com/muhammadasifkham/liberary-management-system`


