#ifndef LIBRARYSYSTEM_HPP
#define LIBRARYSYSTEM_HPP

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <ctime>
#include "../config.hpp"
#include "Book.hpp"
#include "Student.hpp"
#include "Transaction.hpp"
#include "BookRequest.hpp"
#include "../data_structures/BST.hpp"
#include "../data_structures/LinkedList.hpp"
#include "../data_structures/Queue.hpp"
#include "../utils/FileManager.hpp"

/**
 * @struct BorrowResult
 * @brief Result of a book borrowing operation
 */
struct BorrowResult {
    bool success;
    std::string message;
    std::string transactionId;
    std::string dueDate;
    
    BorrowResult(bool s = false, const std::string& m = "", 
                const std::string& tid = "", const std::string& dd = "")
        : success(s), message(m), transactionId(tid), dueDate(dd) {}
};

/**
 * @struct ReturnResult
 * @brief Result of a book return operation
 */
struct ReturnResult {
    bool success;
    std::string message;
    double fineAmount;
    std::string transactionId;
    
    ReturnResult(bool s = false, const std::string& m = "", 
                double fine = 0.0, const std::string& tid = "")
        : success(s), message(m), fineAmount(fine), transactionId(tid) {}
};

/**
 * @struct RequestResult
 * @brief Result of a book request operation
 */
struct RequestResult {
    bool success;
    std::string message;
    int queuePosition;
    
    RequestResult(bool s = false, const std::string& m = "", int pos = -1)
        : success(s), message(m), queuePosition(pos) {}
};

/**
 * @struct SystemStatistics
 * @brief Overall system statistics
 */
struct SystemStatistics {
    int totalBooks;
    int availableBooks;
    int borrowedBooks;
    int totalStudents;
    int activeStudents;
    int pendingRequests;
    int totalTransactions;
    double totalFinesCollected;
    
    SystemStatistics() : totalBooks(0), availableBooks(0), borrowedBooks(0),
                        totalStudents(0), activeStudents(0), pendingRequests(0),
                        totalTransactions(0), totalFinesCollected(0.0) {}
};

/**
 * @class TransactionHistory
 * @brief Manages transaction history using circular array
 */
class TransactionHistory {
private:
    static const int HISTORY_SIZE = LibraryConfig::HISTORY_SIZE;
    Transaction history[HISTORY_SIZE];
    int startIndex;
    int endIndex;
    int count;
    
public:
    TransactionHistory() : startIndex(0), endIndex(0), count(0) {}
    
    void addTransaction(const Transaction& transaction) {
        history[endIndex] = transaction;
        endIndex = (endIndex + 1) % HISTORY_SIZE;
        
        if (count < HISTORY_SIZE) {
            count++;
        } else {
            startIndex = (startIndex + 1) % HISTORY_SIZE;
        }
    }
    
    std::vector<Transaction> getRecentTransactions(int limit = 10) const {
        std::vector<Transaction> recent;
        if (count == 0) return recent;
        
        int displayCount = std::min(limit, count);
        int currentIndex = (endIndex - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        
        for (int i = 0; i < displayCount; i++) {
            recent.push_back(history[currentIndex]);
            currentIndex = (currentIndex - 1 + HISTORY_SIZE) % HISTORY_SIZE;
            if (i == count - 1) break;
        }
        
        return recent;
    }
    
    std::vector<Transaction> getStudentTransactions(int studentId) const {
        std::vector<Transaction> result;
        
        for (int i = 0; i < count; i++) {
            int index = (startIndex + i) % HISTORY_SIZE;
            if (history[index].getStudentId() == studentId) {
                result.push_back(history[index]);
            }
        }
        
        return result;
    }
    
    std::vector<Transaction> getBookTransactions(int bookId) const {
        std::vector<Transaction> result;
        
        for (int i = 0; i < count; i++) {
            int index = (startIndex + i) % HISTORY_SIZE;
            if (history[index].getBookId() == bookId) {
                result.push_back(history[index]);
            }
        }
        
        return result;
    }
    
    std::vector<Transaction> getAllTransactions() const {
        std::vector<Transaction> result;
        
        for (int i = 0; i < count; i++) {
            int index = (startIndex + i) % HISTORY_SIZE;
            result.push_back(history[index]);
        }
        
        return result;
    }
    
    int size() const { return count; }
    void clear() { startIndex = 0; endIndex = 0; count = 0; }
};

/**
 * @class LibrarySystem
 * @brief Main library management system integrating all components
 * 
 * Singleton pattern ensures only one instance of the system exists.
 */
class LibrarySystem {
private:
    // Data structures
    BST<Book> bookCatalog;
    LinkedList<Student> studentList;
    std::map<int, ArrayQueue<BookRequest>> requestQueues; // bookId -> queue
    TransactionHistory transactionHistory;
    FileManager fileManager;
    
    // Helper to get fileManager
    FileManager& getFileManager() { return fileManager; }
    
    // Private constructor for singleton
    LibrarySystem() {}
    
    // Prevent copying
    LibrarySystem(const LibrarySystem&) = delete;
    LibrarySystem& operator=(const LibrarySystem&) = delete;

public:
    /**
     * @brief Get singleton instance
     */
    static LibrarySystem& getInstance() {
        static LibrarySystem instance;
        return instance;
    }
    
    // ========== BOOK MANAGEMENT ==========
    
    /**
     * @brief Add a new book to the catalog
     */
    bool addBook(const Book& book) {
        try {
            return bookCatalog.insert(book);
        } catch (const std::exception& e) {
            std::cerr << "Error adding book: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief Search book by ID
     */
    Book* searchBookById(int bookId) {
        return bookCatalog.search(bookId);
    }
    
    /**
     * @brief Search books by title
     */
    std::vector<Book> searchBookByTitle(const std::string& title) {
        return bookCatalog.searchByTitle(title);
    }
    
    /**
     * @brief Search books by author
     */
    std::vector<Book> searchBookByAuthor(const std::string& author) {
        return bookCatalog.searchByAuthor(author);
    }
    
    /**
     * @brief Update book information
     */
    bool updateBook(int bookId, const Book& updatedBook) {
        return bookCatalog.update(bookId, updatedBook);
    }
    
    /**
     * @brief Remove book from catalog
     */
    bool removeBook(int bookId) {
        // Check if book is currently borrowed
        auto transactions = transactionHistory.getBookTransactions(bookId);
        for (const auto& trans : transactions) {
            if (trans.getType() == BORROW && trans.getReturnDate().empty()) {
                std::cerr << "Cannot remove book: Currently borrowed" << std::endl;
                return false;
            }
        }

        // Safety backup before destructive operation (best-effort)
        (void)fileManager.createBackup("pre_remove_book");

        return bookCatalog.remove(bookId);
    }
    
    /**
     * @brief Get all books
     */
    std::vector<Book> getAllBooks() {
        return bookCatalog.getAllBooks();
    }
    
    /**
     * @brief Display all books
     */
    void displayAllBooks() {
        bookCatalog.displayAll();
    }
    
    // ========== STUDENT MANAGEMENT ==========
    
    /**
     * @brief Register a new student
     */
    bool registerStudent(const Student& student) {
        try {
            return studentList.add(student);
        } catch (const std::exception& e) {
            std::cerr << "Error registering student: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief Search student by ID
     */
    Student* searchStudentById(int studentId) {
        return studentList.findStudent(studentId);
    }
    
    /**
     * @brief Search students by name
     */
    std::vector<Student> searchStudentByName(const std::string& name) {
        return studentList.searchByName(name);
    }
    
    /**
     * @brief Update student information
     */
    bool updateStudent(int studentId, const Student& updatedStudent) {
        return studentList.updateStudent(studentId, updatedStudent);
    }
    
    /**
     * @brief Remove student
     */
    bool removeStudent(int studentId) {
        // Check if student has borrowed books
        Student* student = studentList.findStudent(studentId);
        if (student && student->getBorrowedCount() > 0) {
            std::cerr << "Cannot remove student: Has borrowed books" << std::endl;
            return false;
        }

        // Safety backup before destructive operation (best-effort)
        (void)fileManager.createBackup("pre_remove_student");

        return studentList.removeStudent(studentId);
    }
    
    /**
     * @brief Get all students
     */
    std::vector<Student> getAllStudents() {
        return studentList.getAllStudents();
    }
    
    /**
     * @brief Display all students
     */
    void displayAllStudents() {
        studentList.displayAll();
    }
    
    // ========== BORROWING SYSTEM ==========
    
    /**
     * @brief Issue a book to a student
     */
    BorrowResult issueBook(int studentId, int bookId) {
        // Validate student
        Student* student = studentList.findStudent(studentId);
        if (!student) {
            return BorrowResult(false, "Student not found");
        }
        
        if (!student->getStatus()) {
            return BorrowResult(false, "Student account is inactive");
        }
        
        if (!student->canBorrowMore()) {
            return BorrowResult(false, "Student has reached borrowing limit (3 books)");
        }
        
        // Validate book
        Book* book = bookCatalog.search(bookId);
        if (!book) {
            return BorrowResult(false, "Book not found");
        }
        
        if (!book->isAvailable()) {
            // Add to request queue
            RequestResult reqResult = requestBook(studentId, bookId);
            return BorrowResult(false, "Book not available. " + reqResult.message);
        }
        
        // Perform borrowing
        if (!book->borrowCopy()) {
            return BorrowResult(false, "Failed to borrow book");
        }
        
        if (!student->borrowBook(bookId)) {
            book->returnCopy(); // Rollback
            return BorrowResult(false, "Failed to update student record");
        }
        
        // Create transaction
        Transaction trans(studentId, bookId, BORROW);
        transactionHistory.addTransaction(trans);
        
        return BorrowResult(true, "Book issued successfully", 
                          trans.getTransactionId(), trans.getDueDate());
    }
    
    /**
     * @brief Return a book from a student
     */
    ReturnResult returnBook(int studentId, int bookId) {
        // Validate student
        Student* student = studentList.findStudent(studentId);
        if (!student) {
            return ReturnResult(false, "Student not found");
        }
        
        if (!student->hasBook(bookId)) {
            return ReturnResult(false, "Student does not have this book");
        }
        
        // Validate book
        Book* book = bookCatalog.search(bookId);
        if (!book) {
            return ReturnResult(false, "Book not found");
        }
        
        // Find original borrow transaction
        auto transactions = transactionHistory.getStudentTransactions(studentId);
        Transaction* borrowTrans = nullptr;
        for (auto& trans : transactions) {
            if (trans.getBookId() == bookId && 
                trans.getType() == BORROW && 
                trans.getReturnDate().empty()) {
                borrowTrans = &trans;
                break;
            }
        }
        
        // Calculate fine if overdue
        double fine = 0.0;
        if (borrowTrans && borrowTrans->isOverdue()) {
            fine = borrowTrans->calculateFine();
        }
        
        // Perform return
        if (!book->returnCopy()) {
            return ReturnResult(false, "Failed to return book");
        }
        
        if (!student->returnBook(bookId)) {
            book->borrowCopy(); // Rollback
            return ReturnResult(false, "Failed to update student record");
        }
        
        // Create return transaction
        Transaction returnTrans(studentId, bookId, RETURN);
        returnTrans.setFineAmount(fine);
        transactionHistory.addTransaction(returnTrans);
        
        // Process request queue if exists
        if (requestQueues.find(bookId) != requestQueues.end() && 
            !requestQueues[bookId].isEmpty()) {
            
            BookRequest nextRequest;
            if (requestQueues[bookId].dequeue(nextRequest)) {
                std::cout << "✓ Auto-processing request for Student " 
                         << nextRequest.getStudentId() << std::endl;
                
                BorrowResult borrowResult = issueBook(nextRequest.getStudentId(), bookId);
                if (borrowResult.success) {
                    nextRequest.markFulfilled();
                }
            }
        }
        
        std::string message = "Book returned successfully";
        if (fine > 0) {
            std::ostringstream oss;
            oss << message << ". Fine: $" << std::fixed 
                << std::setprecision(2) << fine;
            message = oss.str();
        }
        
        return ReturnResult(true, message, fine, returnTrans.getTransactionId());
    }
    
    // ========== REQUEST QUEUE SYSTEM ==========
    
    /**
     * @brief Request a book (add to queue)
     */
    RequestResult requestBook(int studentId, int bookId, int priority = 3) {
        // Validate inputs
        Student* student = studentList.findStudent(studentId);
        if (!student) {
            return RequestResult(false, "Student not found");
        }
        
        Book* book = bookCatalog.search(bookId);
        if (!book) {
            return RequestResult(false, "Book not found");
        }
        
        // Check if already in queue
        if (requestQueues.find(bookId) != requestQueues.end()) {
            int position = requestQueues[bookId].getPosition(studentId, bookId);
            if (position > 0) {
                return RequestResult(false, "Already in queue at position " + 
                                   std::to_string(position), position);
            }
        }
        
        // Create queue if doesn't exist
        if (requestQueues.find(bookId) == requestQueues.end()) {
            requestQueues[bookId] = std::move(ArrayQueue<BookRequest>(100));
        }
        
        // Add request
        BookRequest request(studentId, bookId, priority);
        if (requestQueues[bookId].enqueue(request)) {
            int position = requestQueues[bookId].size();
            return RequestResult(true, "Added to queue at position " + 
                               std::to_string(position), position);
        }
        
        return RequestResult(false, "Failed to add to queue");
    }
    
    /**
     * @brief Cancel a book request
     */
    bool cancelRequest(int studentId, int bookId) {
        if (requestQueues.find(bookId) == requestQueues.end()) {
            return false;
        }
        
        BookRequest tempRequest(studentId, bookId);
        return requestQueues[bookId].cancelRequest(studentId, bookId);
    }
    
    /**
     * @brief Get queue for a book
     */
    std::vector<BookRequest> getQueueForBook(int bookId) {
        if (requestQueues.find(bookId) == requestQueues.end()) {
            return {};
        }
        
        std::vector<BookRequest> result;
        auto& q = requestQueues[bookId];
        BookRequest item;
        while(q.dequeue(item)) result.push_back(item);
        for(const auto& r : result) q.enqueue(r);
        return result;
    }
    
    /**
     * @brief Get student's position in queue
     */
    int getStudentQueuePosition(int studentId, int bookId) {
        if (requestQueues.find(bookId) == requestQueues.end()) {
            return -1;
        }
        
        BookRequest tempRequest(studentId, bookId);
        return requestQueues[bookId].getPosition(studentId, bookId);
    }
    
    // ========== TRANSACTION HISTORY ==========
    
    /**
     * @brief Get recent transactions
     */
    std::vector<Transaction> getRecentTransactions(int limit = 10) {
        return transactionHistory.getRecentTransactions(limit);
    }
    
    /**
     * @brief Get student's transaction history
     */
    std::vector<Transaction> getStudentHistory(int studentId) {
        return transactionHistory.getStudentTransactions(studentId);
    }
    
    /**
     * @brief Get book's transaction history
     */
    std::vector<Transaction> getBookHistory(int bookId) {
        return transactionHistory.getBookTransactions(bookId);
    }
    
    // ========== STATISTICS ==========
    
    /**
     * @brief Get system statistics
     */
    SystemStatistics getStatistics() {
        SystemStatistics stats;
        
        auto books = bookCatalog.getAllBooks();
        stats.totalBooks = books.size();
        
        for (const auto& book : books) {
            stats.availableBooks += book.getAvailableCount();
            stats.borrowedBooks += (book.getTotalQuantity() - book.getAvailableCount());
        }
        
        auto students = studentList.getAllStudents();
        stats.totalStudents = students.size();
        
        for (const auto& student : students) {
            if (student.getStatus()) stats.activeStudents++;
        }
        
        for (const auto& pair : requestQueues) {
            stats.pendingRequests += pair.second.size();
        }
        
        stats.totalTransactions = transactionHistory.size();
        
        // Calculate total fines
        auto allTrans = transactionHistory.getAllTransactions();
        for (const auto& trans : allTrans) {
            stats.totalFinesCollected += trans.getFineAmount();
        }
        
        return stats;
    }
    
    /**
     * @brief Get most popular books
     */
    std::vector<std::pair<Book, int>> getMostPopularBooks(int limit = 5) {
        auto books = bookCatalog.getAllBooks();
        std::vector<std::pair<Book, int>> bookBorrows;
        
        for (const auto& book : books) {
            bookBorrows.push_back({book, book.getTimesBorrowed()});
        }
        
        // Sort by times borrowed
        std::sort(bookBorrows.begin(), bookBorrows.end(),
                 [](const auto& a, const auto& b) {
                     return a.second > b.second;
                 });
        
        if (bookBorrows.size() > static_cast<size_t>(limit)) {
            bookBorrows.resize(limit);
        }
        
        return bookBorrows;
    }
    
    // ========== FILE OPERATIONS ==========
    
    /**
     * @brief Save all data to files
     */
    bool saveAllData() {
        std::cout << "\n=== Saving all data ===" << std::endl;

        // Safety backup (best-effort) before writing new snapshots
        (void)fileManager.createBackup("pre_save");
        
        bool success = true;
        success &= getFileManager().saveBooks(bookCatalog);
        success &= getFileManager().saveStudents(studentList);
        success &= getFileManager().saveTransactions(transactionHistory);
        
        // Save requests
        std::vector<BookRequest> allRequests;
        for (auto& pair : requestQueues) {
            std::vector<BookRequest> requests;
            auto& q = pair.second;
            BookRequest req;
            while(q.dequeue(req)) requests.push_back(req);
            for(const auto& r : requests) q.enqueue(r);
            allRequests.insert(allRequests.end(), requests.begin(), requests.end());
        }
        success &= getFileManager().saveRequests(allRequests);
        
        std::cout << (success ? "✓ All data saved successfully" : 
                              "❌ Some data failed to save") << std::endl;
        return success;
    }
    
    /**
     * @brief Load all data from files
     */
    bool loadAllData() {
        std::cout << "\n=== Loading all data ===" << std::endl;
        
        bool success = true;
        success &= getFileManager().loadBooks(bookCatalog);
        success &= getFileManager().loadStudents(studentList);
        // Transactions are optional but should be loaded when present
        success &= getFileManager().loadTransactions(transactionHistory);
        
        // Load requests
        std::vector<BookRequest> allRequests;
        // Requests are best-effort: missing/corrupt file shouldn't prevent startup
        (void)getFileManager().loadRequests(allRequests);
        
        // If any core load failed, reset to a clean state (avoid partial loads)
        if (!success) {
            clearAllData();
            return false;
        }

        // Rebuild request queues
        for (const auto& req : allRequests) {
            if (req.getStatus() == PENDING) {
                int bookId = req.getBookId();
                if (requestQueues.find(bookId) == requestQueues.end()) {
                    requestQueues[bookId] = std::move(ArrayQueue<BookRequest>(100));
                }
                requestQueues[bookId].enqueue(req);
            }
        }
        
        return success;
    }
    
    /**
     * @brief Clear all data
     */
    void clearAllData() {
        bookCatalog.clear();
        studentList.clear();
        requestQueues.clear();
        transactionHistory.clear();
    }
    
    /**
     * @brief Get total book count
     */
    int getTotalBookCount() const {
        return bookCatalog.size();
    }
    
    /**
     * @brief Get available book count
     */
    int getAvailableBookCount() const {
        auto books = bookCatalog.getAllBooks();
        int count = 0;
        for (const auto& book : books) {
            count += book.getAvailableCount();
        }
        return count;
    }
    
    // ========== DATA EXPORT ==========
    
    /**
     * @brief Export books to CSV file
     * @param filename Path to output file
     * @return true if successful
     */
    bool exportBooksToCSV(const std::string& filename) const {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) return false;
            
            // Header
            file << "ID,Title,Author,ISBN,Total Quantity,Available,Times Borrowed,Date Added\n";
            
            // Data
            auto books = bookCatalog.getAllBooks();
            for (const auto& book : books) {
                file << book.getId() << ","
                     << "\"" << book.getTitle() << "\","
                     << "\"" << book.getAuthor() << "\","
                     << book.getIsbn() << ","
                     << book.getTotalQuantity() << ","
                     << book.getAvailableCount() << ","
                     << book.getTimesBorrowed() << ","
                     << book.getDateAdded() << "\n";
            }
            
            file.close();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Export error: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief Export students to CSV file
     * @param filename Path to output file
     * @return true if successful
     */
    bool exportStudentsToCSV(const std::string& filename) const {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) return false;
            
            // Header
            file << "ID,Name,Department,Email,Phone,Borrowed Books,Total Borrowed,Active\n";
            
            // Data
            auto students = studentList.getAllStudents();
            for (const auto& student : students) {
                std::string borrowedBooks;
                auto books = student.getBorrowedBooks();
                for (size_t i = 0; i < books.size(); ++i) {
                    borrowedBooks += std::to_string(books[i]);
                    if (i < books.size() - 1) borrowedBooks += ";";
                }
                
                file << student.getId() << ","
                     << "\"" << student.getName() << "\","
                     << "\"" << student.getDepartment() << "\","
                     << "\"" << student.getEmail() << "\","
                     << "\"" << student.getPhone() << "\","
                     << "\"" << borrowedBooks << "\","
                     << student.getTotalBorrowed() << ","
                     << (student.getStatus() ? "Yes" : "No") << "\n";
            }
            
            file.close();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Export error: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief Export transaction history to CSV file
     * @param filename Path to output file
     * @param limit Maximum number of recent transactions (0 = all)
     * @return true if successful
     */
    bool exportTransactionsToCSV(const std::string& filename, int limit = 0) const {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) return false;
            
            // Header
            file << "Transaction ID,Type,Student ID,Book ID,Issue Date,Due Date,Return Date,Fine,Status\n";
            
            // Data
            auto transactions = transactionHistory.getRecentTransactions(limit > 0 ? limit : 1000);
            for (const auto& trans : transactions) {
                file << trans.getTransactionId() << ","
                     << trans.getTypeString() << ","
                     << trans.getStudentId() << ","
                     << trans.getBookId() << ","
                     << trans.getIssueDate() << ","
                     << trans.getDueDate() << ","
                     << trans.getReturnDate() << ","
                     << trans.getFine() << ","
                     << (trans.isReturned() ? "Returned" : 
                        (trans.isOverdue() ? "Overdue" : "Active")) << "\n";
            }
            
            file.close();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Export error: " << e.what() << std::endl;
            return false;
        }
    }
    
    /**
     * @brief Export statistics report to text file
     * @param filename Path to output file
     * @return true if successful
     */
    bool exportStatisticsReport(const std::string& filename) const {
        try {
            std::ofstream file(filename);
            if (!file.is_open()) return false;
            
            auto stats = getStatistics();
            auto popularBooks = getMostPopularBooks(10);
            
            file << "============================================\n";
            file << "  LIBRARY MANAGEMENT SYSTEM - STATISTICS   \n";
            file << "============================================\n\n";
            
            // Get current date
            time_t now = time(nullptr);
            file << "Report Generated: " << ctime(&now) << "\n";
            
            file << "\n--- BOOKS ---\n";
            file << "Total Books: " << stats.totalBooks << "\n";
            file << "Available Copies: " << stats.availableBooks << "\n";
            file << "Borrowed Copies: " << (stats.totalBooks - stats.availableBooks) << "\n";
            
            file << "\n--- STUDENTS ---\n";
            file << "Total Students: " << stats.totalStudents << "\n";
            file << "Active Students: " << stats.activeStudents << "\n";
            
            file << "\n--- TRANSACTIONS ---\n";
            file << "Total Transactions: " << stats.totalTransactions << "\n";
            file << "Pending Requests: " << stats.pendingRequests << "\n";
            file << "Total Fines Collected: $" << stats.totalFines << "\n";
            
            file << "\n--- TOP 10 POPULAR BOOKS ---\n";
            int rank = 1;
            for (const auto& [book, count] : popularBooks) {
                file << rank++ << ". \"" << book.getTitle() << "\" by " 
                     << book.getAuthor() << " (" << count << " borrows)\n";
            }
            
            file << "\n============================================\n";
            
            file.close();
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Export error: " << e.what() << std::endl;
            return false;
        }
    }
};

#endif // LIBRARYSYSTEM_HPP