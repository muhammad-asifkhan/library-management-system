#ifndef BOOKREQUEST_HPP
#define BOOKREQUEST_HPP

#include <string>
#include <ctime>

enum RequestStatus { PENDING, FULFILLED, CANCELLED, EXPIRED };

class BookRequest {
private:
    int requestId;
    int studentId;
    int bookId;
    time_t requestTime;
    time_t expiryTime;
    RequestStatus status;
    int priority;
    static int nextRequestId;
public:
    BookRequest() : requestId(0), studentId(0), bookId(0), requestTime(0), expiryTime(0), status(PENDING), priority(3) {}
    BookRequest(int studentId, int bookId, int priority = 3)
        : studentId(studentId), bookId(bookId), status(PENDING), priority(priority) {
        requestId = nextRequestId++;
        requestTime = time(nullptr);
        expiryTime = requestTime + 30 * 24 * 60 * 60;
    }
    int getRequestId() const { return requestId; }
    int getStudentId() const { return studentId; }
    int getBookId() const { return bookId; }
    time_t getRequestTime() const { return requestTime; }
    RequestStatus getStatus() const { return status; }
    int getPriority() const { return priority; }
    bool isExpired() const { return time(nullptr) > expiryTime; }
    void setStatus(RequestStatus newStatus) { status = newStatus; }
    void setPriority(int newPriority) { priority = newPriority; }
    void markFulfilled() { status = FULFILLED; }
    std::string toString() const;
    std::string toCSV() const;
    static BookRequest fromCSV(const std::string& csvLine);
    bool operator==(const BookRequest& other) const { return studentId == other.studentId && bookId == other.bookId; }
};

#endif // BOOKREQUEST_HPP
