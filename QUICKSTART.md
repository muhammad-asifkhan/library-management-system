# 🚀 Quick Start (GUI + Login)

This project is **GUI-first** (Dear ImGui + SFML) and starts with a **login screen** (Student / Librarian).

## Build

### Linux/macOS (recommended)

```bash
cd /home/muhammad-asif-khan/liberary-management-system
chmod +x scripts/create_build.sh
./scripts/create_build.sh
```

This builds to `build-gui/`.

### Manual (CMake)

```bash
cmake -S . -B build-gui -DCMAKE_BUILD_TYPE=Release
cmake --build build-gui -j4
```

## Run

```bash
./build-gui/library_gui
```

## Login (Demo Credentials)

- **Librarian**: `admin` / `admin123`
- **Student**:
  - `100001` with PIN `0001`
  - `100002` with PIN `0002`

PIN rule: **last 4 digits of Student ID**.

## Permissions (Important)

- **Librarian**: full CRUD + issue/return + save/reload
- **Student**: can **return** and **request** books + use **Assistant** tab; no CRUD and no issuing
Enter your choice:
```

### Quick Test Workflow:

1. **Press 5** → Display all books (5 sample books loaded)
2. **Press 0** → Back to main menu
3. **Press 2** → Student Management
4. **Press 4** → Display all students (3 sample students)
5. **Press 0** → Back to main menu
6. **Press 3** → Issue/Return Books
7. **Press 1** → Issue Book
   - Enter Student ID: `100001`
   - Enter Book ID: `1001`
   - See success message with due date!
8. **Press 2** → Return Book
   - Enter same Student ID and Book ID
   - See return confirmation
9. **Press 6** → View statistics
10. **Press 0** → Exit (data auto-saves)

## 🎯 What You Get

### Sample Data Included:

**5 Books:**
- 1001: The Great Gatsby by F. Scott Fitzgerald
- 1002: 1984 by George Orwell
- 1003: To Kill a Mockingbird by Harper Lee
- 1004: Pride and Prejudice by Jane Austen
- 1005: The Catcher in the Rye by J.D. Salinger

**3 Students:**
- 100001: John Doe (Computer Science)
- 100002: Jane Smith (Mathematics)
- 100003: Bob Johnson (Physics)

### All Features Working:
✅ Add/search/update/remove books  
✅ Register/search students  
✅ Issue/return books with fine calculation  
✅ Transaction history  
✅ Request queue  
✅ Statistics and reports  
✅ File persistence (auto-save)  
✅ Data validation  

## 🔧 Troubleshooting

### "Compiler not found"
**Solution**: Install g++ or clang++
```bash
# Ubuntu/Debian
sudo apt-get install g++

# macOS
xcode-select --install

# Windows
# Download MinGW from mingw-w64.org
```

### "C++17 not supported"
**Solution**: Update your compiler
```bash
# Check version
g++ --version  # Should be 7.0+

# Ubuntu: upgrade
sudo apt-get install g++-9
```

### "Permission denied" (Linux/Mac)
**Solution**: Make scripts executable
```bash
chmod +x build.sh
```

### Build errors
**Solution**: Manual compile with verbose output
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build -j4
```

## 📁 File Structure After First Run

After running the application, you'll see:

```
LibraryManagementSystem/
├── build/
│   └── library_gui          (executable)
├── data/
│   ├── books.csv            (book data)
│   ├── students.csv         (student data)
│   ├── transactions.csv     (transaction history)
│   ├── requests.csv         (book requests)
│   └── backups/             (safety backups)
└── [source files...]
```

## 🎓 Next Steps

1. **Read the README.md** for complete documentation
2. **Check PROJECT_REPORT.md** for technical details
3. **Modify config.hpp** to customize policies
4. **Run tests** to verify everything works
5. **Start adding your own data!**

## 💡 Pro Tips

### Quick Commands:

```bash
# Build and run in one command
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j4 && ./build/library_gui

# Run tests only
ctest --test-dir build --output-on-failure

# Clean build
rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j4

# Backup data (built-in)
# (run via app: it saves CSVs; backups can also be created by copying data/)
```

### Keyboard Shortcuts:
- `0` - Back/Exit from any menu
- `Ctrl+C` - Emergency exit (Linux/Mac)
- `Ctrl+Z` - Emergency exit (Windows)

### Data Files:
- All data saved in `data/` folder
- CSV format - can open in Excel/LibreOffice
- Backups: Copy `data/` folder (or use the FileManager backup helpers programmatically)

## ⚡ Performance Tips

### For Large Datasets:

1. **Increase history size** in `config.hpp`:
```cpp
constexpr int HISTORY_SIZE = 5000;  // Default is 1000
```

2. **Optimize for search**:
   - Use ID search (O(log n))
   - Avoid title/author search on large catalogs (O(n))

3. **Batch operations**:
   - Add multiple books/students before saving
   - Save manually instead of auto-save for bulk imports

## 🆘 Need Help?

### Common Questions:

**Q: Can I import my own book data?**  
A: Yes! Edit `data/books.csv` following the format, or add via menu option 1.

**Q: How do I reset the system?**  
A: Delete the `data/` folder and restart. Sample data will be recreated.

**Q: Can I change the borrowing limit?**  
A: Yes! Edit `MAX_BOOKS_PER_STUDENT` in `config.hpp` and rebuild.

**Q: Where are backups stored?**  
A: `data/backups/` folder, created automatically.

**Q: How do I export data?**  
A: Copy files from `data/` folder. They're standard CSV format.

### Still Stuck?

1. Check the full **README.md**
2. Read **PROJECT_REPORT.md** for technical details
3. Review test output: `ctest --test-dir build --output-on-failure`
4. Check compiler output for specific errors

## ✨ You're All Set!

Congratulations! You now have a fully functional Library Management System.

**Enjoy managing your library! 📚**

---

*Total setup time: ~5 minutes*  
*Lines of code: ~5000+*  
*Test suite: run via CTest (`BUILD_TESTS=ON`)*  
*Ready for production: ✓*