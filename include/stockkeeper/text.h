#ifndef STOCKKEEPER_TEXT_H
#define STOCKKEEPER_TEXT_H

#include <string>
#include <vector>

namespace text {
std::string trim(const std::string& s);
std::string to_lower(std::string s);
std::vector<std::string> split(const std::string& line, char delimiter);

std::vector<std::string> tokenize(const std::string& line);

int parse_int(const std::string& s);
double parse_double(const std::string& s);

}

#endif
