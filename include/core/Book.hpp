#ifndef BOOK_HPP
#define BOOK_HPP

#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <regex>

/**
 * @class Book
 * @brief Represents a book in the library system
 * 
 * Manages book information including availability, borrowing statistics,
 * and provides validation for all book data.
 */
class Book {
private:
    // MANDATORY fields
    int id;                             // Unique identifier
    std::string title;                   // Book title
    std::string author;                  // Author name
    std::string isbn;                    // ISBN-13 format
    int totalQuantity;                   // Total copies owned
    int availableCount;                  // Currently available copies
    
    // ADDITIONAL fields
    std::string categories;               // Semicolon-separated categories
    std::string publisher;               // Publisher name
    int publicationYear;                 // Year published
    std::string location;                // Physical location in library
    time_t dateAdded;                   // When added to system
    int timesBorrowed;                  // Total times borrowed
    bool isReference;                   // Reference only (cannot be borrowed)
    
    // Validation constants
    static constexpr int MIN_ID = 1;
    static constexpr int MAX_ID = 999999;
    static constexpr int MIN_QUANTITY = 1;
    static constexpr int MIN_YEAR = 1900;
    static constexpr int MAX_YEAR = 2025;

public:
    /**
     * @brief Default constructor
     */
    Book() : id(0), totalQuantity(0), availableCount(0), 
             publicationYear(2025), dateAdded(time(nullptr)),
             timesBorrowed(0), isReference(false) {}
    
    /**
     * @brief Parameterized constructor with validation
     */
    Book(int id, const std::string& title, const std::string& author,
         const std::string& isbn, int quantity)
        : id(id), title(title), author(author), isbn(isbn),
          totalQuantity(quantity), availableCount(quantity),
          publicationYear(2025), dateAdded(time(nullptr)),
          timesBorrowed(0), isReference(false) {
        
        validateId(id);
        validateTitle(title);
        validateAuthor(author);
        validateISBN(isbn);
        validateQuantity(quantity);
    }
    
    // Getters
    int getId() const { return id; }
    const std::string& getTitle() const { return title; }
    const std::string& getAuthor() const { return author; }
    const std::string& getIsbn() const { return isbn; }
    int getTotalQuantity() const { return totalQuantity; }
    int getAvailableCount() const { return availableCount; }
    int getTimesBorrowed() const { return timesBorrowed; }
    bool getIsReference() const { return isReference; }
    time_t getDateAdded() const { return dateAdded; }
    
    // Setters with validation
    void setTitle(const std::string& newTitle) {
        validateTitle(newTitle);
        title = newTitle;
    }
    
    void setAuthor(const std::string& newAuthor) {
        validateAuthor(newAuthor);
        author = newAuthor;
    }
    
    void setISBN(const std::string& newIsbn) {
        validateISBN(newIsbn);
        isbn = newIsbn;
    }
    
    void setTotalQuantity(int quantity) {
        validateQuantity(quantity);
        totalQuantity = quantity;
        if (availableCount > quantity) {
            availableCount = quantity;
        }
    }
    
    /**
     * @brief Check if book is available for borrowing
     */
    bool isAvailable() const {
        return availableCount > 0 && !isReference;
    }
    
    /**
     * @brief Borrow a copy of the book
     */
    bool borrowCopy() {
        if (isAvailable()) {
            availableCount--;
            timesBorrowed++;
            return true;
        }
        return false;
    }
    
    /**
     * @brief Return a copy of the book
     */
    bool returnCopy() {
        if (availableCount < totalQuantity) {
            availableCount++;
            return true;
        }
        return false;
    }
    
    /**
     * @brief Convert book to string representation
     */
    std::string toString() const {
        std::stringstream ss;
        ss << "ID: " << id << "\n";
        ss << "Title: " << title << "\n";
        ss << "Author: " << author << "\n";
        ss << "ISBN: " << isbn << "\n";
        ss << "Available: " << availableCount << "/" << totalQuantity << "\n";
        ss << "Times Borrowed: " << timesBorrowed << "\n";
        if (isReference) {
            ss << "Type: Reference Only\n";
        }
        return ss.str();
    }
    
    /**
     * @brief Convert to CSV format
     */
    std::string toCSV() const {
        std::stringstream ss;
        ss << id << ","
           << "\"" << title << "\","
           << "\"" << author << "\","
           << isbn << ","
           << totalQuantity << ","
           << availableCount << ","
           << "\"" << categories << "\","
           << "\"" << publisher << "\","
           << publicationYear << ","
           << "\"" << location << "\","
           << dateAdded << ","
           << timesBorrowed << ","
           << isReference;
        return ss.str();
    }
    
    /**
     * @brief Create Book from CSV string
     */
    static Book fromCSV(const std::string& csv) {
        Book book;
        std::vector<std::string> fields;
        bool inQuotes = false;
        std::string field;
        
        for (size_t i = 0; i < csv.length(); i++) {
            char c = csv[i];
            
            if (c == '"') {
                if (i + 1 < csv.length() && csv[i + 1] == '"') {
                    // Escaped quote
                    field += '"';
                    i++; // Skip next quote
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
        fields.push_back(field); // Add last field
        
        if (fields.size() >= 12) {
            try {
                book.id = std::stoi(fields[0]);
                book.title = fields[1];
                book.author = fields[2];
                book.isbn = fields[3];
                book.totalQuantity = std::stoi(fields[4]);
                book.availableCount = std::stoi(fields[5]);
                book.categories = fields[6];
                book.publisher = fields[7];
                book.publicationYear = std::stoi(fields[8]);
                book.location = fields[9];
                book.dateAdded = std::stoll(fields[10]);
                book.timesBorrowed = std::stoi(fields[11]);
                book.isReference = (fields.size() > 12) ? (fields[12] == "1" || fields[12] == "true") : false;
            } catch (const std::exception& e) {
                throw std::runtime_error("Error parsing CSV data: " + std::string(e.what()));
            }
        } else {
            throw std::runtime_error("Invalid CSV format: insufficient fields");
        }
        
        return book;
    }
    
    // Comparison operators for BST
    bool operator<(const Book& other) const {
        return id < other.id;
    }
    
    bool operator==(const Book& other) const {
        return id == other.id;
    }
    
    bool operator!=(const Book& other) const {
        return id != other.id;
    }

private:
    /**
     * @brief Validate book ID
     */
    void validateId(int id) const {
        if (id < MIN_ID || id > MAX_ID) {
            throw std::invalid_argument("Book ID must be between " + 
                                     std::to_string(MIN_ID) + " and " + 
                                     std::to_string(MAX_ID));
        }
    }
    
    /**
     * @brief Validate title
     */
    void validateTitle(const std::string& title) const {
        if (title.empty() || title.length() > 500) {
            throw std::invalid_argument("Title cannot be empty and must be <= 500 characters");
        }
    }
    
    /**
     * @brief Validate author
     */
    void validateAuthor(const std::string& author) const {
        if (author.empty() || author.length() > 200) {
            throw std::invalid_argument("Author cannot be empty and must be <= 200 characters");
        }
    }
    
    /**
     * @brief Validate ISBN (must be 13 digits)
     */
    void validateISBN(const std::string& isbn) const {
        std::regex isbnRegex("^\\d{13}$");
        if (!std::regex_match(isbn, isbnRegex)) {
            throw std::invalid_argument("ISBN must be exactly 13 digits");
        }
    }
    
    /**
     * @brief Validate quantity
     */
    void validateQuantity(int quantity) const {
        if (quantity < MIN_QUANTITY) {
            throw std::invalid_argument("Quantity must be at least " + 
                                     std::to_string(MIN_QUANTITY));
        }
    }
};

#endif // BOOK_HPP