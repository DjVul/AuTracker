#include "Sale.h"
#include <cctype>
#include <stdexcept>
#include <sstream>

// Constructor
Sale::Sale(const std::string& owner,
    double grams,
    double buyPricePerGram,
    double sellPricePerGram,
    Currency currency,
    const std::string& buyDate,
    const std::string& sellDate)
    : owner(owner),
    grams(grams),
    buyPricePerGram(buyPricePerGram),
    sellPricePerGram(sellPricePerGram),
    currency(currency),
    buyDate(buyDate),
    sellDate(sellDate)
{
    if (owner.empty())
        throw std::invalid_argument("Owner cannot be empty.");
    if (grams <= 0)
        throw std::invalid_argument("Grams must be positive.");
    if (buyPricePerGram <= 0)
        throw std::invalid_argument("Buy price must be positive.");
    if (sellPricePerGram <= 0)
        throw std::invalid_argument("Sell price must be positive.");
    if (!isValidCurrency(currency))
        throw std::invalid_argument("Invalid currency.");
    if (!buyDate.empty() && !isValidDate(buyDate))
        throw std::invalid_argument("Invalid buy date format. Expected DD-MM-YYYY.");
    if (!sellDate.empty() && !isValidDate(sellDate))
        throw std::invalid_argument("Invalid sell date format. Expected DD-MM-YYYY.");
}

// Getters
std::string Sale::getOwner() const { return owner; }
double Sale::getGrams() const { return grams; }
double Sale::getBuyPricePerGram() const { return buyPricePerGram; }
double Sale::getSellPricePerGram() const { return sellPricePerGram; }
Sale::Currency Sale::getCurrency() const { return currency; }
std::string Sale::getBuyDate() const { return buyDate; }
std::string Sale::getSellDate() const { return sellDate; }

// Business logic
double Sale::getBuyTotal() const { return grams * buyPricePerGram; }
double Sale::getSellTotal() const { return grams * sellPricePerGram; }
double Sale::getProfit() const { return getSellTotal() - getBuyTotal(); }

double Sale::getProfitPercent() const
{
    double buyTotal = getBuyTotal();
    if (buyTotal == 0) return 0.0;
    return (getProfit() / buyTotal) * 100.0;
}

// Serialization
std::string Sale::serialize() const
{
    return owner + "," +
        std::to_string(grams) + "," +
        std::to_string(buyPricePerGram) + "," +
        std::to_string(sellPricePerGram) + "," +
        currencyToString(currency) + "," +
        buyDate + "," +
        sellDate;
}

Sale Sale::deserialize(const std::string& line)
{
    std::stringstream ss(line);
    std::string ownerStr, gramsStr, buyPriceStr, sellPriceStr, currencyStr, buyDateStr, sellDateStr;

    std::getline(ss, ownerStr, ',');
    std::getline(ss, gramsStr, ',');
    std::getline(ss, buyPriceStr, ',');
    std::getline(ss, sellPriceStr, ',');
    std::getline(ss, currencyStr, ',');
    std::getline(ss, buyDateStr, ',');
    std::getline(ss, sellDateStr, ',');

    if (ownerStr.empty() || gramsStr.empty() || buyPriceStr.empty() ||
        sellPriceStr.empty() || currencyStr.empty())
        throw std::invalid_argument("Invalid line: missing fields.");

    double grams = std::stod(gramsStr);
    double buyPrice = std::stod(buyPriceStr);
    double sellPrice = std::stod(sellPriceStr);

    Currency currency = stringToCurrency(currencyStr);
    if (!isValidCurrency(currency))
        throw std::invalid_argument("Invalid currency in line: " + line);

    return Sale(ownerStr, grams, buyPrice, sellPrice, currency, buyDateStr, sellDateStr);
}

// Helper functions
bool Sale::isValidCurrency(Currency currency)
{
    switch (currency)
    {
    case Currency::RSD:
    case Currency::USD:
    case Currency::EUR:
    case Currency::GBP:
    case Currency::JPY:
    case Currency::CAD:
        return true;
    default:
        return false;
    }
}

bool Sale::isValidDate(const std::string& date)
{
    size_t firstDash = date.find('-');
    if (firstDash == std::string::npos) return false;
    size_t secondDash = date.find('-', firstDash + 1);
    if (secondDash == std::string::npos) return false;
    if (date.find('-', secondDash + 1) != std::string::npos) return false;

    std::string dayStr = date.substr(0, firstDash);
    std::string monthStr = date.substr(firstDash + 1, secondDash - firstDash - 1);
    std::string yearStr = date.substr(secondDash + 1);

    if (dayStr.empty() || monthStr.empty() || yearStr.empty()) return false;
    for (char c : dayStr) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    for (char c : monthStr) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    for (char c : yearStr) if (!std::isdigit(static_cast<unsigned char>(c))) return false;

    if (yearStr.length() != 4) return false;
    if (dayStr.length() > 2 || monthStr.length() > 2) return false;

    int day = std::stoi(dayStr);
    int month = std::stoi(monthStr);
    int year = std::stoi(yearStr);

    if (month < 1 || month > 12) return false;
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (isLeap) daysInMonth[1] = 29;
    if (day < 1 || day > daysInMonth[month - 1]) return false;
    return true;
}

std::string Sale::currencyToString(Currency currency)
{
    switch (currency)
    {
    case Currency::RSD: return "RSD";
    case Currency::USD: return "USD";
    case Currency::EUR: return "EUR";
    case Currency::GBP: return "GBP";
    case Currency::JPY: return "JPY";
    case Currency::CAD: return "CAD";
    default: return "UNKNOWN";
    }
}

Sale::Currency Sale::stringToCurrency(const std::string& str)
{
    if (str == "RSD") return Currency::RSD;
    if (str == "USD") return Currency::USD;
    if (str == "EUR") return Currency::EUR;
    if (str == "GBP") return Currency::GBP;
    if (str == "JPY") return Currency::JPY;
    if (str == "CAD") return Currency::CAD;
    return static_cast<Currency>(-1);
}