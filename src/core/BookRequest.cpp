#include "../../include/core/BookRequest.hpp"
#include <sstream>

int BookRequest::nextRequestId = 1;

std::string BookRequest::toString() const {
    std::stringstream ss;
    ss << "RQ" << requestId << " | Student: " << studentId << " | Book: " << bookId
       << " | Priority: " << priority << " | Status: " << status;
    return ss.str();
}

std::string BookRequest::toCSV() const {
    std::stringstream ss;
    ss << requestId << ',' << studentId << ',' << bookId << ',' << requestTime << ','
       << expiryTime << ',' << status << ',' << priority;
    return ss.str();
}

BookRequest BookRequest::fromCSV(const std::string& csvLine) {
    BookRequest req;
    std::stringstream ss(csvLine);
    std::string field;
    int idx = 0;
    while (std::getline(ss, field, ',')) {
        switch (idx) {
            case 0: req.requestId = std::stoi(field); break;
            case 1: req.studentId = std::stoi(field); break;
            case 2: req.bookId = std::stoi(field); break;
            case 3: req.requestTime = std::stoll(field); break;
            case 4: req.expiryTime = std::stoll(field); break;
            case 5: req.status = static_cast<RequestStatus>(std::stoi(field)); break;
            case 6: req.priority = std::stoi(field); break;
        }
        ++idx;
    }
    return req;
}
