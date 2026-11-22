#pragma once

#include <string>

class Sale
{
public:
    enum class Currency { RSD, USD, EUR, GBP, JPY, CAD };

    Sale(const std::string& owner,
        double grams,
        double buyPricePerGram,
        double sellPricePerGram,
        Currency currency,
        const std::string& buyDate,
        const std::string& sellDate);

    // Getters
    std::string getOwner() const;
    double getGrams() const;
    double getBuyPricePerGram() const;
    double getSellPricePerGram() const;
    Currency getCurrency() const;
    std::string getBuyDate() const;
    std::string getSellDate() const;

    // Business logic
    double getBuyTotal() const;
    double getSellTotal() const;
    double getProfit() const;
    double getProfitPercent() const;

    // Serialization
    std::string serialize() const;
    static Sale deserialize(const std::string& line);

    // Helper functions
    static bool isValidCurrency(Currency currency);
    static bool isValidDate(const std::string& date);
    static std::string currencyToString(Currency currency);
    static Currency stringToCurrency(const std::string& str);

private:
    std::string owner;
    double grams;
    double buyPricePerGram;
    double sellPricePerGram;
    Currency currency;
    std::string buyDate;
    std::string sellDate;
};