#include "PriceProvider.h"
#include <curl/curl.h>
#include "json.hpp"
#include <iostream>
#include <map>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* out)
{
    size_t totalSize = size * nmemb;
    out->append((char*)contents, totalSize);
    return totalSize;
}

double PriceProvider::getCurrentPricePerGram(Purchase::Currency currency)
{
    std::string currencyStr;
    switch (currency)
    {
    case Purchase::Currency::RSD: currencyStr = "RSD"; break;
    case Purchase::Currency::USD: currencyStr = "USD"; break;
    case Purchase::Currency::EUR: currencyStr = "EUR"; break;
    case Purchase::Currency::GBP: currencyStr = "GBP"; break;
    case Purchase::Currency::JPY: currencyStr = "JPY"; break;
    case Purchase::Currency::CAD: currencyStr = "CAD"; break;
    default: return -1.0;
    }

    std::string apiUrl = "https://xaus.com/api/v1/spot?unit=gram&currency=" + currencyStr;

    CURL* curl = curl_easy_init();
    if (!curl) return -1.0;

    std::string readBuffer;
    CURLcode res;

    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AuTracker/1.0");

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
        return -1.0;
    }

    try
    {
        auto jsonData = nlohmann::json::parse(readBuffer);

        // Price is nested inside "xau" object
        if (jsonData.contains("xau") && jsonData["xau"].contains("price"))
        {
            return jsonData["xau"]["price"].get<double>();
        }

        std::cerr << "Unexpected JSON: " << readBuffer << std::endl;
        return -1.0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON error: " << e.what() << std::endl;
        return -1.0;
    }
}

double PriceProvider::getExchangeRate(Purchase::Currency from, Purchase::Currency to)
{
    if (from == to) return 1.0;

    // Check cache first (rates change once per day, so cache is safe)
    static std::map<std::pair<int, int>, double> cache;
    auto key = std::make_pair(static_cast<int>(from), static_cast<int>(to));

    auto it = cache.find(key);
    if (it != cache.end())
        return it->second;

    // Map currency enum to string
    std::string fromStr, toStr;
    switch (from) {
    case Purchase::Currency::RSD: fromStr = "RSD"; break;
    case Purchase::Currency::USD: fromStr = "USD"; break;
    case Purchase::Currency::EUR: fromStr = "EUR"; break;
    case Purchase::Currency::GBP: fromStr = "GBP"; break;
    case Purchase::Currency::JPY: fromStr = "JPY"; break;
    case Purchase::Currency::CAD: fromStr = "CAD"; break;
    default: return 1.0;
    }
    switch (to) {
    case Purchase::Currency::RSD: toStr = "RSD"; break;
    case Purchase::Currency::USD: toStr = "USD"; break;
    case Purchase::Currency::EUR: toStr = "EUR"; break;
    case Purchase::Currency::GBP: toStr = "GBP"; break;
    case Purchase::Currency::JPY: toStr = "JPY"; break;
    case Purchase::Currency::CAD: toStr = "CAD"; break;
    default: return 1.0;
    }

    std::string apiUrl = "https://api.frankfurter.dev/v2/rate/" + fromStr + "/" + toStr;

    CURL* curl = curl_easy_init();
    if (!curl) return 1.0;

    std::string readBuffer;
    CURLcode res;

    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AuTracker/1.0");

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        std::cerr << "Exchange rate curl error: "
            << curl_easy_strerror(res) << std::endl;
        return 1.0;
    }

    try
    {
        auto jsonData = nlohmann::json::parse(readBuffer);

        if (jsonData.contains("rate"))
        {
            double rate = jsonData["rate"].get<double>();
            cache[key] = rate;
            return rate;
        }

        std::cerr << "Unexpected exchange rate JSON: "
            << readBuffer << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exchange rate JSON error: "
            << e.what() << std::endl;
    }

    return 1.0;
}

double PriceProvider::convertCurrency(double amount,
    Purchase::Currency from,
    Purchase::Currency to)
{
    if (from == to) return amount;

    double rate = getExchangeRate(from, to);
    return amount * rate;
}

std::vector<std::pair<std::string, double>> PriceProvider::getHistoricalPrices(
    Purchase::Currency currency, int daysBack)
{
    std::vector<std::pair<std::string, double>> result;

    // XAUS history endpoint always returns USD, so fetch USD and convert later
    std::string apiUrl = "https://xaus.com/api/v1/history?currency=USD";

    CURL* curl = curl_easy_init();
    if (!curl) return result;

    std::string readBuffer;
    CURLcode res;

    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "AuTracker/1.0");

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK)
    {
        std::cerr << "Curl error: " << curl_easy_strerror(res) << std::endl;
        return result;
    }

    try
    {
        auto jsonData = nlohmann::json::parse(readBuffer);

        if (jsonData.contains("points") && jsonData["points"].is_array())
        {
            for (const auto& entry : jsonData["points"])
            {
                if (entry.contains("d") && entry.contains("c"))
                {
                    std::string date = entry["d"].get<std::string>();
                    double pricePerOunceUsd = entry["c"].get<double>();

                    // Convert troy ounce to gram: USD per gram
                    double pricePerGramUsd = pricePerOunceUsd / 31.1034768;

                    // Convert USD to requested currency
                    double pricePerGram = convertCurrency(
                        pricePerGramUsd,
                        Purchase::Currency::USD,
                        currency);

                    result.push_back({ date, pricePerGram });
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "JSON error: " << e.what() << std::endl;
    }

    // Keep only the last `daysBack` points
    if (daysBack > 0 && result.size() > static_cast<size_t>(daysBack))
    {
        result.erase(result.begin(), result.end() - daysBack);
    }

    return result;
}