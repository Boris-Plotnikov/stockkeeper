#ifndef STOCKKEEPER_MONEY_H
#define STOCKKEEPER_MONEY_H

#include <string>

namespace money {
long long parse_cents(const std::string& s);

std::string format(long long cents);

}

#endif
