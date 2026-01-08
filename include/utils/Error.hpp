#ifndef ERROR_HPP
#define ERROR_HPP
#include <exception>
#include <string>
#include <ctime>

enum ErrorCode {
    ERROR_NONE = 0,
    ERROR_BOOK_NOT_FOUND,
    ERROR_STUDENT_NOT_FOUND,
    ERROR_BORROW_LIMIT,
    ERROR_BOOK_UNAVAILABLE
};

class LibraryException : public std::exception {
    std::string message;
    ErrorCode code;
    time_t timestamp;
public:
    LibraryException(ErrorCode c, const std::string& msg)
        : message(msg), code(c), timestamp(time(nullptr)) {}
    const char* what() const noexcept override { return message.c_str(); }
    ErrorCode getCode() const { return code; }
};

class BookNotFoundException : public LibraryException {
public:
    BookNotFoundException(int bookId) : LibraryException(ERROR_BOOK_NOT_FOUND, "Book with ID " + std::to_string(bookId) + " not found") {}
};
class StudentNotFoundException : public LibraryException {
public:
    StudentNotFoundException(int studentId) : LibraryException(ERROR_STUDENT_NOT_FOUND, "Student with ID " + std::to_string(studentId) + " not found") {}
};
class BorrowLimitExceededException : public LibraryException {
public:
    BorrowLimitExceededException(int studentId) : LibraryException(ERROR_BORROW_LIMIT, "Student " + std::to_string(studentId) + " exceeded max borrow limit") {}
};
class BookUnavailableException : public LibraryException {
public:
    BookUnavailableException(int bookId) : LibraryException(ERROR_BOOK_UNAVAILABLE, "Book " + std::to_string(bookId) + " is unavailable") {}
};

#endif // ERROR_HPP

