#ifndef PRICEPROVIDER_H
#define PRICEPROVIDER_H

#include "Purchase.h"
#include <vector>
#include <utility>



class PriceProvider
{
public:
    // Returns current gold price per gram for the given currency.
    // Returns -1.0 on error.
    static double getCurrentPricePerGram(Purchase::Currency currency);

    static std::vector<std::pair<std::string, double>> getHistoricalPrices(
        Purchase::Currency currency, int daysBack);

    // Converts amount from one currency to another using hardcoded rates.
    static double convertCurrency(double amount,
        Purchase::Currency from,
        Purchase::Currency to);

    static double getExchangeRate(Purchase::Currency from, Purchase::Currency to);
};

#endif // PRICEPROVIDER_H