#ifndef TRANSACTION_HPP
#define TRANSACTION_HPP

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <vector>
#include "../config.hpp"

enum TransactionType { BORROW, RETURN };

/**
 * @class Transaction
 * @brief Represents a borrowing/returning transaction
 * 
 * Tracks all book borrowing and returning activities with timestamps,
 * due dates, and fine calculations.
 */
class Transaction {
private:
    std::string transactionId;      // Unique identifier (TXYYYYMMDDHHMMSSXXX)
    int studentId;                 // Student involved
    int bookId;                   // Book involved
    TransactionType type;           // BORROW or RETURN
    time_t timestamp;              // When transaction occurred
    std::string issueDate;         // Formatted issue date
    std::string dueDate;          // Expected return date
    std::string returnDate;        // Actual return date (empty if not returned)
    double fineAmount;             // Late fee amount
    int librarianId;               // Librarian who processed
    std::string notes;             // Additional notes

public:
    /**
     * @brief Constructor for new transaction
     */
    Transaction(int studentId, int bookId, TransactionType type, int librarianId = 0)
        : studentId(studentId), bookId(bookId), type(type),
          timestamp(time(nullptr)), fineAmount(0.0), librarianId(librarianId) {
        
        generateTransactionId();
        setFormattedDates();
        
        if (type == BORROW) {
            calculateDueDate();
        }
    }
    
    /**
     * @brief Default constructor
     */
    Transaction() : studentId(0), bookId(0), type(BORROW),
                   timestamp(time(nullptr)), fineAmount(0.0), librarianId(0) {
        generateTransactionId();
    }
    
    // Getters
    const std::string& getTransactionId() const { return transactionId; }
    int getStudentId() const { return studentId; }
    int getBookId() const { return bookId; }
    TransactionType getType() const { return type; }
    time_t getTimestamp() const { return timestamp; }
    const std::string& getIssueDate() const { return issueDate; }
    const std::string& getDueDate() const { return dueDate; }
    const std::string& getReturnDate() const { return returnDate; }
    double getFineAmount() const { return fineAmount; }
    int getLibrarianId() const { return librarianId; }
    const std::string& getNotes() const { return notes; }
    
    // Setters
    void setReturnDate(const std::string& date) { returnDate = date; }
    void setFineAmount(double amount) { fineAmount = amount; }
    void setNotes(const std::string& notes) { this->notes = notes; }
    void setLibrarianId(int id) { librarianId = id; }
    
    /**
     * @brief Mark transaction as returned
     */
    void markReturned() {
        if (type == BORROW && returnDate.empty()) {
            returnDate = getCurrentDateString();
            calculateFine();
        }
    }
    
    /**
     * @brief Check if transaction is overdue
     */
    bool isOverdue() const {
        if (type != BORROW || !returnDate.empty()) {
            return false;
        }
        
        time_t now = time(nullptr);
        time_t dueTime = parseDate(dueDate);
        return now > dueTime;
    }
    
    /**
     * @brief Calculate days overdue
     */
    int getDaysOverdue() const {
        if (!isOverdue()) {
            return 0;
        }
        
        time_t now = time(nullptr);
        time_t dueTime = parseDate(dueDate);
        return (now - dueTime) / (24 * 60 * 60); // Convert seconds to days
    }
    
    /**
     * @brief Calculate fine amount
     */
    double calculateFine() {
        if (type != BORROW) {
            return 0.0;
        }
        
        int daysOverdue = getDaysOverdue();
        fineAmount = std::min(daysOverdue * LibraryConfig::FINE_PER_DAY, 
                            LibraryConfig::MAX_FINE_AMOUNT);
        return fineAmount;
    }
    
    /**
     * @brief Get transaction type as string
     */
    std::string getTypeString() const {
        return (type == BORROW) ? "BORROW" : "RETURN";
    }
    
    /**
     * @brief Convert transaction to string representation
     */
    std::string toString() const {
        std::stringstream ss;
        ss << "Transaction ID: " << transactionId << "\n";
        ss << "Type: " << getTypeString() << "\n";
        ss << "Student ID: " << studentId << "\n";
        ss << "Book ID: " << bookId << "\n";
        ss << "Issue Date: " << issueDate << "\n";
        
        if (type == BORROW) {
            ss << "Due Date: " << dueDate << "\n";
            if (!returnDate.empty()) {
                ss << "Return Date: " << returnDate << "\n";
            }
            if (fineAmount > 0) {
                ss << "Fine Amount: $" << std::fixed << std::setprecision(2) << fineAmount << "\n";
            }
            if (isOverdue()) {
                ss << "Status: OVERDUE (" << getDaysOverdue() << " days)\n";
            } else {
                ss << "Status: Active\n";
            }
        } else {
            ss << "Return Date: " << returnDate << "\n";
        }
        
        return ss.str();
    }
    
    /**
     * @brief Convert to CSV format
     */
    std::string toCSV() const {
        std::stringstream ss;
        ss << transactionId << ","
           << studentId << ","
           << bookId << ","
           << getTypeString() << ","
           << timestamp << ","
           << "\"" << issueDate << "\","
           << "\"" << dueDate << "\","
           << "\"" << returnDate << "\","
           << fineAmount << ","
           << librarianId << ","
           << "\"" << notes << "\"";
        return ss.str();
    }
    
    /**
     * @brief Create Transaction from CSV string
     */
    static Transaction fromCSV(const std::string& csv) {
        Transaction transaction;
        std::stringstream ss(csv);
        std::string field;
        std::vector<std::string> fields;
        
        while (std::getline(ss, field, ',')) {
            if (field.front() == '"' && field.back() == '"') {
                field = field.substr(1, field.length() - 2);
            }
            fields.push_back(field);
        }
        
        if (fields.size() >= 11) {
            transaction.transactionId = fields[0];
            transaction.studentId = std::stoi(fields[1]);
            transaction.bookId = std::stoi(fields[2]);
            transaction.type = (fields[3] == "BORROW") ? BORROW : RETURN;
            transaction.timestamp = std::stoll(fields[4]);
            transaction.issueDate = fields[5];
            transaction.dueDate = fields[6];
            transaction.returnDate = fields[7];
            transaction.fineAmount = std::stod(fields[8]);
            transaction.librarianId = std::stoi(fields[9]);
            transaction.notes = fields[10];
        }
        
        return transaction;
    }

private:
    /**
     * @brief Generate unique transaction ID
     */
    void generateTransactionId() {
        std::stringstream ss;
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        
        ss << "TX"
           << std::setfill('0') << std::setw(4) << (timeinfo->tm_year + 1900)
           << std::setfill('0') << std::setw(2) << (timeinfo->tm_mon + 1)
           << std::setfill('0') << std::setw(2) << timeinfo->tm_mday
           << std::setfill('0') << std::setw(2) << timeinfo->tm_hour
           << std::setfill('0') << std::setw(2) << timeinfo->tm_min
           << std::setfill('0') << std::setw(2) << timeinfo->tm_sec
           << std::setfill('0') << std::setw(3) << (now % 1000);
        
        transactionId = ss.str();
    }
    
    /**
     * @brief Set formatted date strings
     */
    void setFormattedDates() {
        issueDate = getCurrentDateString();
        if (type == BORROW) {
            dueDate = calculateDueDateString();
        }
    }
    
    /**
     * @brief Get current date as string
     */
    std::string getCurrentDateString() const {
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(4) << (timeinfo->tm_year + 1900) << "-"
           << std::setfill('0') << std::setw(2) << (timeinfo->tm_mon + 1) << "-"
           << std::setfill('0') << std::setw(2) << timeinfo->tm_mday;
        return ss.str();
    }
    
    /**
     * @brief Calculate due date string
     */
    void calculateDueDate() {
        time_t dueTime = timestamp + (LibraryConfig::LOAN_PERIOD_DAYS * 24 * 60 * 60);
        struct tm* timeinfo = localtime(&dueTime);
        
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(4) << (timeinfo->tm_year + 1900) << "-"
           << std::setfill('0') << std::setw(2) << (timeinfo->tm_mon + 1) << "-"
           << std::setfill('0') << std::setw(2) << timeinfo->tm_mday;
        dueDate = ss.str();
    }
    
    /**
     * @brief Calculate due date string for constructor
     */
    std::string calculateDueDateString() const {
        time_t dueTime = timestamp + (LibraryConfig::LOAN_PERIOD_DAYS * 24 * 60 * 60);
        struct tm* timeinfo = localtime(&dueTime);
        
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(4) << (timeinfo->tm_year + 1900) << "-"
           << std::setfill('0') << std::setw(2) << (timeinfo->tm_mon + 1) << "-"
           << std::setfill('0') << std::setw(2) << timeinfo->tm_mday;
        return ss.str();
    }
    
    /**
     * @brief Parse date string to time_t
     */
    time_t parseDate(const std::string& dateStr) const {
        struct tm timeinfo = {};
        std::istringstream ss(dateStr);
        char dash;
        
        ss >> timeinfo.tm_year >> dash >> timeinfo.tm_mon >> dash >> timeinfo.tm_mday;
        timeinfo.tm_year -= 1900;
        timeinfo.tm_mon -= 1;
        timeinfo.tm_hour = 0;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        
        return mktime(&timeinfo);
    }
};

#endif // TRANSACTION_HPP
