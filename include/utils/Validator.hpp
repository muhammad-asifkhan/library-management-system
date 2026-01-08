#ifndef VALIDATOR_HPP
#define VALIDATOR_HPP

#include <string>
#include <regex>

namespace Validator {
    inline bool isValidISBN(const std::string& isbn) {
        return std::regex_match(isbn, std::regex("^\\d{13}$"));
    }
    inline bool isValidEmail(const std::string& email) {
        return std::regex_match(email, std::regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,})"));
    }
    inline bool isValidYear(int year) {
        return year >= 1900 && year <= 2024;
    }
    inline bool isValidStudentId(int id) {
        return id >= 100000 && id <= 999999;
    }
}

#endif // VALIDATOR_HPP

