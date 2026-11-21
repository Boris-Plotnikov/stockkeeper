#include "stockkeeper/money.h"

#include <cctype>
#include <stdexcept>

#include "stockkeeper/text.h"

namespace money {
long long parse_cents(const std::string& input) {
    std::string s = text::trim(input);
    if (s.empty()) throw std::invalid_argument("Price is empty");
    std::size_t dot = s.find('.');
    std::string whole = dot == std::string::npos ? s : s.substr(0, dot);
    std::string fraction = dot == std::string::npos ? "" : s.substr(dot + 1);
    if (whole.empty() || fraction.size() > 2) throw std::invalid_argument("Bad price: '" + s + "'");
    for (char c : whole + fraction) {
        if (!std::isdigit(static_cast<unsigned char>(c))) throw std::invalid_argument("Bad price: '" + s + "'");
    }
    while (fraction.size() < 2) fraction += '0';
    return std::stoll(whole) * 100 + std::stoll(fraction);
}

std::string format(long long cents) {
    std::string sign = cents < 0 ? "-" : "";
    long long abs_cents = cents < 0 ? -cents : cents;
    std::string fraction = std::to_string(abs_cents % 100);
    if (fraction.size() < 2) fraction = "0" + fraction;
    return sign + std::to_string(abs_cents / 100) + "." + fraction;
}

}
