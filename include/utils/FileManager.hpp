#ifndef FILEMANAGER_HPP
#define FILEMANAGER_HPP
#include <string>
#include <vector>
#include "../core/Book.hpp"
#include "../core/Student.hpp"
#include "../core/Transaction.hpp"
#include "../core/BookRequest.hpp"
#include "../data_structures/BST.hpp"
#include "../data_structures/LinkedList.hpp"

class TransactionHistory;
class FileManager {
    std::string dataDirectory;
public:
    FileManager(const std::string& basePath="data/") : dataDirectory(basePath) {}
    // Save/load
    bool saveBooks(const BST<Book>& bookCatalog);
    bool loadBooks(BST<Book>& bookCatalog);
    bool saveStudents(const LinkedList<Student>& studentList);
    bool loadStudents(LinkedList<Student>& studentList);
    bool saveTransactions(const TransactionHistory& history);
    bool loadTransactions(TransactionHistory& history);
    bool saveRequests(const std::vector<BookRequest>& reqs);
    bool loadRequests(std::vector<BookRequest>& reqs);
    // Config/INI
    bool saveConfig(const std::string& iniFile="settings.ini");
    bool loadConfig(const std::string& iniFile="settings.ini");
    // Backup/restore
    bool createBackup(const std::string& backupName="");
    bool restoreBackup(const std::string& backupName);
    // Integrity
    bool verifyDataIntegrity();
};
#endif // FILEMANAGER_HPP
