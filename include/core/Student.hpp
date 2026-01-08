#ifndef STUDENT_HPP
#define STUDENT_HPP

#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <regex>

/**
 * @class Student
 * @brief Represents a student in the library system
 * 
 * Manages student information, borrowing records, and enforces
 * the 3-book borrowing limit.
 */
class Student {
private:
    // MANDATORY fields
    int studentId;                      // 6-digit unique ID (100000-999999)
    std::string name;                   // Full name
    std::string department;             // Academic department
    std::vector<int> borrowedBooks;     // Current book IDs
    
    // ADDITIONAL fields
    std::string email;                  // Email address
    std::string phone;                  // Phone number
    std::string address;                // Physical address
    time_t registrationDate;            // When registered
    bool isActive;                      // Account status
    int totalBooksBorrowed;             // Lifetime count
    
    // Constants
    static constexpr int MAX_BORROW_LIMIT = 3;
    static constexpr int MIN_STUDENT_ID = 100000;
    static constexpr int MAX_STUDENT_ID = 999999;

public:
    /**
     * @brief Default constructor
     */
    Student() : studentId(0), registrationDate(time(nullptr)),
                isActive(true), totalBooksBorrowed(0) {}
    
    /**
     * @brief Parameterized constructor with validation
     */
    Student(int id, const std::string& name, const std::string& dept,
            const std::string& email = "", const std::string& phone = "")
        : studentId(id), name(name), department(dept), email(email),
          phone(phone), registrationDate(time(nullptr)),
          isActive(true), totalBooksBorrowed(0) {
        
        // Validate inputs
        if (!isValidStudentId(id)) {
            throw std::invalid_argument("Student ID must be 6 digits (100000-999999)");
        }
        if (name.empty()) {
            throw std::invalid_argument("Student name cannot be empty");
        }
        if (department.empty()) {
            throw std::invalid_argument("Department cannot be empty");
        }
        if (!email.empty() && !isValidEmail(email)) {
            throw std::invalid_argument("Invalid email format");
        }
    }
    
    // GETTERS
    int getId() const { return studentId; }
    std::string getName() const { return name; }
    std::string getDepartment() const { return department; }
    std::string getEmail() const { return email; }
    std::string getPhone() const { return phone; }
    std::string getAddress() const { return address; }
    time_t getRegistrationDate() const { return registrationDate; }
    bool getStatus() const { return isActive; }
    int getTotalBooksBorrowed() const { return totalBooksBorrowed; }
    std::vector<int> getBorrowedBooks() const { return borrowedBooks; }
    int getBorrowedCount() const { return borrowedBooks.size(); }
    
    // SETTERS
    void setName(const std::string& newName) {
        if (newName.empty()) {
            throw std::invalid_argument("Name cannot be empty");
        }
        name = newName;
    }
    
    void setDepartment(const std::string& dept) {
        if (dept.empty()) {
            throw std::invalid_argument("Department cannot be empty");
        }
        department = dept;
    }
    
    void setEmail(const std::string& newEmail) {
        if (!newEmail.empty() && !isValidEmail(newEmail)) {
            throw std::invalid_argument("Invalid email format");
        }
        email = newEmail;
    }
    
    void setPhone(const std::string& newPhone) { phone = newPhone; }
    void setAddress(const std::string& addr) { address = addr; }
    
    // CRITICAL BORROWING METHODS
    /**
     * @brief Check if student can borrow more books
     * @return true if student has capacity to borrow more books
     */
    bool canBorrowMore() const {
        return borrowedBooks.size() < MAX_BORROW_LIMIT && isActive;
    }
    
    /**
     * @brief Borrow a book
     * @param bookId The ID of the book to borrow
     * @return true if successful, false if limit reached or already has book
     */
    bool borrowBook(int bookId) {
        if (!isActive) {
            throw std::runtime_error("Student account is inactive");
        }
        
        if (borrowedBooks.size() >= MAX_BORROW_LIMIT) {
            return false; // Limit reached
        }
        
        if (hasBook(bookId)) {
            return false; // Already has this book
        }
        
        borrowedBooks.push_back(bookId);
        totalBooksBorrowed++;
        return true;
    }
    
    /**
     * @brief Return a book
     * @param bookId The ID of the book to return
     * @return true if successful, false if student doesn't have this book
     */
    bool returnBook(int bookId) {
        auto it = std::find(borrowedBooks.begin(), borrowedBooks.end(), bookId);
        if (it != borrowedBooks.end()) {
            borrowedBooks.erase(it);
            return true;
        }
        return false; // Student doesn't have this book
    }
    
    /**
     * @brief Check if student currently has a specific book
     * @param bookId The book ID to check
     * @return true if student has the book
     */
    bool hasBook(int bookId) const {
        return std::find(borrowedBooks.begin(), borrowedBooks.end(), bookId) 
               != borrowedBooks.end();
    }
    
    // STUDENT MANAGEMENT
    /**
     * @brief Activate student account
     */
    void activate() { isActive = true; }
    
    /**
     * @brief Deactivate student account
     */
    void deactivate() { isActive = false; }
    
    // UTILITY METHODS
    /**
     * @brief Convert student to display string
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << "Student ID: " << studentId << "\n"
            << "Name: " << name << "\n"
            << "Department: " << department << "\n"
            << "Email: " << email << "\n"
            << "Phone: " << phone << "\n"
            << "Status: " << (isActive ? "ACTIVE" : "INACTIVE") << "\n"
            << "Books Borrowed: " << borrowedBooks.size() << "/" << MAX_BORROW_LIMIT << "\n"
            << "Total Lifetime Borrows: " << totalBooksBorrowed;
        
        if (!borrowedBooks.empty()) {
            oss << "\nCurrently Borrowed Book IDs: ";
            for (size_t i = 0; i < borrowedBooks.size(); i++) {
                oss << borrowedBooks[i];
                if (i < borrowedBooks.size() - 1) oss << ", ";
            }
        }
        
        return oss.str();
    }
    
    /**
     * @brief Convert student to CSV format
     * Format: student_id,name,department,email,phone,registration_date,is_active,borrowed_books,total_borrowed
     */
    std::string toCSV() const {
        std::ostringstream oss;
        
        // Join borrowed books with semicolon
        std::string borrowedStr;
        for (size_t i = 0; i < borrowedBooks.size(); i++) {
            borrowedStr += std::to_string(borrowedBooks[i]);
            if (i < borrowedBooks.size() - 1) borrowedStr += ";";
        }
        
        oss << studentId << ","
            << "\"" << name << "\","
            << "\"" << department << "\","
            << "\"" << email << "\","
            << "\"" << phone << "\","
            << registrationDate << ","
            << (isActive ? 1 : 0) << ","
            << "\"" << borrowedStr << "\","
            << totalBooksBorrowed;
        
        return oss.str();
    }
    
    /**
     * @brief Create Student object from CSV line
     */
    static Student fromCSV(const std::string& csvLine) {
        std::vector<std::string> fields;
        bool inQuotes = false;
        std::string field;
        
        for (char c : csvLine) {
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                fields.push_back(field);
                field.clear();
            } else {
                field += c;
            }
        }
        fields.push_back(field);
        
        if (fields.size() < 9) {
            throw std::runtime_error("Invalid CSV format");
        }
        
        Student student;
        student.studentId = std::stoi(fields[0]);
        student.name = fields[1];
        student.department = fields[2];
        student.email = fields[3];
        student.phone = fields[4];
        student.registrationDate = std::stoll(fields[5]);
        student.isActive = (std::stoi(fields[6]) == 1);
        
        // Parse borrowed books
        std::string borrowedStr = fields[7];
        if (!borrowedStr.empty()) {
            std::istringstream bookStream(borrowedStr);
            std::string bookIdStr;
            while (std::getline(bookStream, bookIdStr, ';')) {
                if (!bookIdStr.empty()) {
                    student.borrowedBooks.push_back(std::stoi(bookIdStr));
                }
            }
        }
        
        student.totalBooksBorrowed = std::stoi(fields[8]);
        
        return student;
    }
    
    // VALIDATION METHODS
    /**
     * @brief Validate student ID (must be 6 digits)
     */
    static bool isValidStudentId(int id) {
        return id >= MIN_STUDENT_ID && id <= MAX_STUDENT_ID;
    }
    
    /**
     * @brief Validate email format
     */
    static bool isValidEmail(const std::string& email) {
        // Simple email validation
        const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        return std::regex_match(email, pattern);
    }
    
    /**
     * @brief Check if all student fields are valid
     */
    bool isValid() const {
        return isValidStudentId(studentId) &&
               !name.empty() &&
               !department.empty() &&
               borrowedBooks.size() <= MAX_BORROW_LIMIT &&
               (email.empty() || isValidEmail(email));
    }
    
    // Get max borrow limit (static)
    static int getMaxBorrowLimit() { return MAX_BORROW_LIMIT; }
};

#endif // STUDENT_HPP