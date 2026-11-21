#include "stockkeeper/text.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace text {
std::string trim(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : line) {
        if (c == delimiter) {
            parts.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    parts.push_back(current);
    return parts;
}

std::vector<std::string> tokenize(const std::string& line) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    bool has_token = false;
    for (char c : line) {
        if (c == '"') {
            in_quotes = !in_quotes;
            has_token = true;
        } else if (std::isspace(static_cast<unsigned char>(c)) && !in_quotes) {
            if (has_token) {
                tokens.push_back(current);
                current.clear();
                has_token = false;
            }
        } else {
            current += c;
            has_token = true;
        }
    }
    if (in_quotes) throw std::invalid_argument("Missing closing quote");
    if (has_token) tokens.push_back(current);
    return tokens;
}

int parse_int(const std::string& s) {
    std::size_t used = 0;
    int value = 0;
    try {
        value = std::stoi(s, &used);
    } catch (const std::exception&) {
        throw std::invalid_argument("Not a whole number: '" + s + "'");
    }
    if (used != s.size()) throw std::invalid_argument("Not a whole number: '" + s + "'");
    return value;
}

double parse_double(const std::string& s) {
    std::size_t used = 0;
    double value = 0;
    try {
        value = std::stod(s, &used);
    } catch (const std::exception&) {
        throw std::invalid_argument("Not a number: '" + s + "'");
    }
    if (used != s.size()) throw std::invalid_argument("Not a number: '" + s + "'");
    return value;
}

}
