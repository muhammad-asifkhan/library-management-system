#include "../../include/utils/FileManager.hpp"
#include "../../include/core/LibrarySystem.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>

namespace {
namespace fs = std::filesystem;

fs::path normalizeDir(const std::string& dir) {
    fs::path p(dir);
    if (p.empty()) return fs::path("data");
    return p;
}

bool ensureDirExists(const fs::path& dir) {
    std::error_code ec;
    fs::create_directories(dir, ec);
    return !ec;
}

bool atomicWriteTextFile(const fs::path& target, const std::string& content) {
    if (!ensureDirExists(target.parent_path())) return false;

    fs::path tmp = target;
    tmp += ".tmp";

    {
        std::ofstream out(tmp, std::ios::trunc);
        if (!out.is_open()) return false;
        out << content;
        out.flush();
        if (!out.good()) return false;
    }

    std::error_code ec;
    if (fs::exists(target, ec) && !ec) fs::remove(target, ec);
    ec.clear();
    fs::rename(tmp, target, ec);
    if (ec) {
        // Best-effort cleanup
        std::error_code ec2;
        fs::remove(tmp, ec2);
        return false;
    }
    return true;
}

std::string nowTimestampForName() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}
} // namespace

bool FileManager::saveBooks(const BST<Book>& bookCatalog) {
    const auto dir = normalizeDir(dataDirectory);
    std::ostringstream oss;
    for (const auto& b : bookCatalog.getAllBooks()) {
        oss << b.toCSV() << "\n";
    }
    return atomicWriteTextFile(dir / "books.csv", oss.str());
}
bool FileManager::loadBooks(BST<Book>& bookCatalog) {
    const auto dir = normalizeDir(dataDirectory);
    std::ifstream file(dir / "books.csv");
    if (!file.is_open()) return false;

    bookCatalog.clear();
    std::string l;
    int loadedCount = 0;
    int errorCount = 0;

    auto splitCsv = [](const std::string& csvLine) -> std::vector<std::string> {
        std::vector<std::string> fields;
        bool inQuotes = false;
        std::string field;
        for (size_t i = 0; i < csvLine.size(); i++) {
            char c = csvLine[i];
            if (c == '"') {
                if (inQuotes && i + 1 < csvLine.size() && csvLine[i + 1] == '"') {
                    field += '"';
                    i++;
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }
        fields.push_back(field);
        return fields;
    };

    while (std::getline(file, l)) {
        if (l.empty()) continue;

        // Header support
        if (l.rfind("id,", 0) == 0) continue;

        try {
            // Preferred (new) format: Book::toCSV() / Book::fromCSV()
            bookCatalog.insert(Book::fromCSV(l));
            loadedCount++;
            continue;
        } catch (const std::exception&) {
            // Backward-compatible fallback:
            // Old format supported: id,title,author,isbn,quantity
            try {
                const auto fields = splitCsv(l);
                if (fields.size() < 5) {
                    errorCount++;
                    continue;
                }
                const int id = std::stoi(fields[0]);
                const std::string title = fields[1];
                const std::string author = fields[2];
                const std::string isbn = fields[3];
                const int qty = std::stoi(fields[4]);
                bookCatalog.insert(Book(id, title, author, isbn, qty));
                loadedCount++;
            } catch (const std::exception&) {
                errorCount++;
                continue;
            }
        }
    }

    if (errorCount > 0) {
        std::cerr << "Warning: loadBooks skipped " << errorCount << " invalid row(s).\n";
    }

    // Don't fail the entire app startup because of partially bad data.
    // If file existed but contained no valid rows, we still consider load successful.
    (void)verifyDataIntegrity();
    return true;
}
bool FileManager::saveStudents(const LinkedList<Student>& studentList) {
    const auto dir = normalizeDir(dataDirectory);
    std::ostringstream oss;
    for (const auto& s : studentList.getAllStudents()) {
        oss << s.toCSV() << "\n";
    }
    return atomicWriteTextFile(dir / "students.csv", oss.str());
}
bool FileManager::loadStudents(LinkedList<Student>& studentList) {
    const auto dir = normalizeDir(dataDirectory);
    std::ifstream f(dir / "students.csv");
    if (!f.is_open()) return false;

    studentList.clear();
    std::string line;
    try {
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            if (line.rfind("student_id,", 0) == 0) continue; // header support
            studentList.add(Student::fromCSV(line));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading students: " << e.what() << std::endl;
        return false;
    }
    return true;
}
bool FileManager::saveTransactions(const TransactionHistory& history) {
    const auto dir = normalizeDir(dataDirectory);
    std::ostringstream oss;
    for (const auto& t : history.getAllTransactions()) {
        oss << t.toCSV() << "\n";
    }
    return atomicWriteTextFile(dir / "transactions.csv", oss.str());
}

bool FileManager::loadTransactions(TransactionHistory& history) {
    const auto dir = normalizeDir(dataDirectory);
    std::ifstream f(dir / "transactions.csv");
    if (!f.is_open()) return false;

    history.clear();
    std::string line;
    try {
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            if (line.rfind("transaction_id,", 0) == 0) continue; // header support
            history.addTransaction(Transaction::fromCSV(line));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading transactions: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool FileManager::saveRequests(const std::vector<BookRequest>& reqs) {
    const auto dir = normalizeDir(dataDirectory);
    std::ostringstream oss;
    for (const auto& req : reqs) {
        oss << req.toCSV() << "\n";
    }
    return atomicWriteTextFile(dir / "requests.csv", oss.str());
}
bool FileManager::loadRequests(std::vector<BookRequest>& reqs) {
    const auto dir = normalizeDir(dataDirectory);
    std::ifstream f(dir / "requests.csv");
    if (!f.is_open()) return false;

    reqs.clear();
    std::string line;
    try {
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            if (line.rfind("request_id,", 0) == 0) continue; // header support
            reqs.push_back(BookRequest::fromCSV(line));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading requests: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool FileManager::saveConfig(const std::string& iniFile) {
    const auto dir = normalizeDir(dataDirectory);
    const auto target = dir / iniFile;

    // Minimal INI-style config: key=value
    std::ostringstream oss;
    oss << "DATA_DIRECTORY=" << dir.string() << "\n";
    oss << "MAX_BOOKS_PER_STUDENT=" << LibraryConfig::MAX_BOOKS_PER_STUDENT << "\n";
    oss << "LOAN_PERIOD_DAYS=" << LibraryConfig::LOAN_PERIOD_DAYS << "\n";
    oss << "FINE_PER_DAY=" << LibraryConfig::FINE_PER_DAY << "\n";
    oss << "MAX_FINE_AMOUNT=" << LibraryConfig::MAX_FINE_AMOUNT << "\n";
    oss << "REQUEST_EXPIRY_DAYS=" << LibraryConfig::REQUEST_EXPIRY_DAYS << "\n";
    oss << "HISTORY_SIZE=" << LibraryConfig::HISTORY_SIZE << "\n";

    return atomicWriteTextFile(target, oss.str());
}

bool FileManager::loadConfig(const std::string& iniFile) {
    // Config values are compile-time constants right now; we only validate file exists and is readable.
    const auto dir = normalizeDir(dataDirectory);
    std::ifstream f(dir / iniFile);
    return f.is_open();
}

bool FileManager::createBackup(const std::string& backupName) {
    const auto dir = normalizeDir(dataDirectory);
    if (!ensureDirExists(dir)) return false;

    const fs::path backupsDir = dir / "backups";
    if (!ensureDirExists(backupsDir)) return false;

    const std::string name = backupName.empty() ? nowTimestampForName() : backupName;
    const fs::path dst = backupsDir / name;
    if (!ensureDirExists(dst)) return false;

    // Copy known data files if they exist
    const fs::path files[] = {
        dir / "books.csv",
        dir / "students.csv",
        dir / "transactions.csv",
        dir / "requests.csv",
        dir / "settings.ini"
    };

    std::error_code ec;
    for (const auto& src : files) {
        if (fs::exists(src, ec) && !ec) {
            ec.clear();
            fs::copy_file(src, dst / src.filename(), fs::copy_options::overwrite_existing, ec);
            if (ec) return false;
        }
    }

    // Keep only the newest N backups (best-effort)
    std::vector<fs::directory_entry> backups;
    ec.clear();
    for (const auto& entry : fs::directory_iterator(backupsDir, ec)) {
        if (ec) break;
        if (entry.is_directory()) backups.push_back(entry);
    }

    std::sort(backups.begin(), backups.end(),
              [](const fs::directory_entry& a, const fs::directory_entry& b) {
                  std::error_code e1, e2;
                  return fs::last_write_time(a, e1) > fs::last_write_time(b, e2);
              });

    for (size_t i = LibraryConfig::BACKUP_COUNT; i < backups.size(); i++) {
        std::error_code rmEc;
        fs::remove_all(backups[i].path(), rmEc);
    }

    return true;
}

bool FileManager::restoreBackup(const std::string& backupName) {
    const auto dir = normalizeDir(dataDirectory);
    const fs::path backupsDir = dir / "backups";
    const fs::path srcDir = backupsDir / backupName;
    if (!fs::exists(srcDir)) return false;

    if (!ensureDirExists(dir)) return false;

    std::error_code ec;
    for (auto& entry : fs::directory_iterator(srcDir, ec)) {
        if (ec) return false;
        if (!entry.is_regular_file()) continue;
        ec.clear();
        fs::copy_file(entry.path(), dir / entry.path().filename(),
                      fs::copy_options::overwrite_existing, ec);
        if (ec) return false;
    }
    return true;
}

bool FileManager::verifyDataIntegrity() {
    // Minimal integrity checks: directory exists and no obvious corruption (files readable).
    const auto dir = normalizeDir(dataDirectory);
    return ensureDirExists(dir);
}
