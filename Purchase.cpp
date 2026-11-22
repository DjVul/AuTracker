#include "Purchase.h"

#include <cctype>     // for std::isdigit
#include <stdexcept>  // for std::invalid_argument
#include <iostream>   // for std::cout (print)
#include <sstream>    // for std::stringstream (deserialize)
#include <string>     // for std::to_string, std::stod


Purchase::Purchase(const std::string& owner, double grams, double pricePerGram, Currency currency, const std::string& date): 
	owner(owner),
	grams(grams),
	pricePerGram(pricePerGram),
	currency(currency),
	date(date)
{
	if (owner.empty())
	{
		throw std::invalid_argument("Owner cannot be empty.");
	}
	if (grams <= 0)
	{
		throw std::invalid_argument("Grams cannot be negative or zero.");
	}
	if (pricePerGram <= 0)
	{
		throw std::invalid_argument("Price per gram cannot be negative or zero.");
	}
	if (!isValidCurrency(currency))
	{
		throw std::invalid_argument("Invalid currency.");
	}
	if (!date.empty() && !isValidDate(date))
	{
		throw std::invalid_argument("Invalid date format. Expected format: DD-MM-YYYY.");
	}

}

// Getters
std::string Purchase::getOwner() const
{
	return this->owner;
}

std::string Purchase::getDate() const
{
	return this->date;
}

double Purchase::getGrams() const
{
	return this->grams;
}

double Purchase::getPricePerGram() const
{
	return this->pricePerGram;
}

Purchase::Currency Purchase::getCurrency() const
{
	return this->currency;
}


// Setters

void Purchase::setOwner(const std::string& owner)
{
	if(owner.empty())
	{
		throw std::invalid_argument("Owner cannot be empty.");
	}
	this->owner = owner;
}

void Purchase::setDate(const std::string& date)
{
	if (!date.empty() && !isValidDate(date))
	{
		throw std::invalid_argument("Invalid date format. Expected format: DD-MM-YYYY.");
	}
	this->date = date;
}

void Purchase::setGrams(double grams)
{
	if (grams <= 0)
	{
		throw std::invalid_argument("Grams cannot be negative or zero.");
	}
	this->grams = grams;
}

void Purchase::setPricePerGram(double pricePerGram)
{
	if (pricePerGram <= 0)
	{
		throw std::invalid_argument("Price per gram cannot be negative or zero.");
	}
	this->pricePerGram = pricePerGram;
}

void Purchase::setCurrency(Currency currency)
{
	if (!isValidCurrency(currency))
	{
		throw std::invalid_argument("Invalid currency.");
	}
	this->currency = currency;
}

bool Purchase::isValidCurrency(const Currency currency)
{
	switch(currency)
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

bool Purchase::isValidDate(const std::string& date)
{
	// 1. Must have exactly two '-' separators
	size_t firstDash = date.find('-');
	if (firstDash == std::string::npos) return false;

	size_t secondDash = date.find('-', firstDash + 1);
	if (secondDash == std::string::npos) return false;

	// 2. No more than two dashes
	if (date.find('-', secondDash + 1) != std::string::npos) return false;

	// 3. Split into three parts
	std::string dayStr = date.substr(0, firstDash);
	std::string monthStr = date.substr(firstDash + 1, secondDash - firstDash - 1);
	std::string yearStr = date.substr(secondDash + 1);
	
	// 4. Each part must be non-empty and contain only digits
	if (dayStr.empty() || monthStr.empty() || yearStr.empty()) return false;

	for (char c : dayStr)   if (!std::isdigit(static_cast<unsigned char>(c))) return false;
	for (char c : monthStr) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
	for (char c : yearStr)  if (!std::isdigit(static_cast<unsigned char>(c))) return false;

	// 5. Year must be exactly 4 digits
	if (yearStr.length() != 4) return false;

	// 6. Day and month: 1 or 2 digits
	if (dayStr.length() > 2 || monthStr.length() > 2) return false;

	// 7. Convert to numbers
	int day = std::stoi(dayStr);
	int month = std::stoi(monthStr);
	int year = std::stoi(yearStr);

	// 8. Month 1-12
	if (month < 1 || month > 12) return false;

	// 9. Days in each month
	int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	// 10. Leap year — February has 29 days
	bool isLeap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
	if (isLeap) daysInMonth[1] = 29;

	// 11. Day must be within valid range
	if (day < 1 || day > daysInMonth[month - 1]) return false;

	return true;
}

double Purchase::getTotalPrice() const
{
	return this->grams * this->pricePerGram;
}

std::string Purchase::currencyToString(Currency currency)
{
	switch (currency)
	{
		case Currency::RSD: return "RSD";
		case Currency::USD: return "USD";
		case Currency::EUR: return "EUR";
		case Currency::GBP: return "GBP";
		case Currency::JPY: return "JPY";
		case Currency::CAD: return "CAD";
		default:            return "UNKNOWN";
	}
}

Purchase::Currency Purchase::stringToCurrency(const std::string& currencyStr)
{
	if (currencyStr == "RSD") return Currency::RSD;
	if (currencyStr == "USD") return Currency::USD;
	if (currencyStr == "EUR") return Currency::EUR;
	if (currencyStr == "GBP") return Currency::GBP;
	if (currencyStr == "JPY") return Currency::JPY;
	if (currencyStr == "CAD") return Currency::CAD;
	return static_cast<Currency>(-1); // Default
}

void Purchase::print() const
{
	std::cout << "Owner: " << owner << "\n";
	std::cout << "Date: " << (date.empty() ? "N/A" : date) << "\n";
	std::cout << "Grams: " << grams << " g\n";
	std::cout << "Price per gram: " << pricePerGram << " " << currencyToString(currency) << "\n";
	std::cout << "Total cost: " << getTotalPrice() << " " << currencyToString(currency) << "\n";
}

std::string Purchase::serialize() const
{
	return owner + "," +
		std::to_string(grams) + "," +
		std::to_string(pricePerGram) + "," +
		currencyToString(currency) + "," +
		date;
}

Purchase Purchase::deserialize(const std::string& data)
{
	std::stringstream ss(data);
	std::string ownerStr, gramsStr, priceStr, currencyStr, dateStr;

	std::getline(ss, ownerStr, ',');
	std::getline(ss, gramsStr, ',');
	std::getline(ss, priceStr, ',');
	std::getline(ss, currencyStr, ',');
	std::getline(ss, dateStr, ',');

	// Validate that all required fields are present
	if (ownerStr.empty())
		throw std::invalid_argument("Invalid line: missing owner.");
	if (gramsStr.empty())
		throw std::invalid_argument("Invalid line: missing grams.");
	if (priceStr.empty())
		throw std::invalid_argument("Invalid line: missing price per gram.");
	if (currencyStr.empty())
		throw std::invalid_argument("Invalid line: missing currency.");

	// Parse numeric values (std::stod throws on invalid input)
	double grams = std::stod(gramsStr);
	double pricePerGram = std::stod(priceStr);

	// Parse currency
	Currency currency = stringToCurrency(currencyStr);
	if (!isValidCurrency(currency))
		throw std::invalid_argument("Invalid currency in line: " + data);

	// Date is optional — empty is allowed
	// Constructor will validate everything else
	return Purchase(ownerStr, grams, pricePerGram, currency, dateStr);
}



