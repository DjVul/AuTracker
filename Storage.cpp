#include "Storage.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

// ============================================================
// Users
// ============================================================

bool Storage::saveUsers(const std::vector<User>& users, const std::string& path)
{
    std::ofstream file(path);
    if (!file.is_open())
        return false;

    for (const auto& user : users)
        file << user.serialize() << "\n";

    return true;
}

std::vector<User> Storage::loadUsers(const std::string& path)
{
    std::vector<User> users;
    std::ifstream file(path);
    if (!file.is_open())
        return users;

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        try
        {
            users.push_back(User::deserialize(line));
        }
        catch (const std::exception& e)
        {
            std::cerr << "Skipping invalid user line: " << e.what() << "\n";
        }
    }

    return users;
}

// ============================================================
// Purchases
// ============================================================

bool Storage::savePurchases(const std::vector<Purchase>& purchases, const std::string& path)
{
    std::ofstream file(path);
    if (!file.is_open())
        return false;

    for (const auto& purchase : purchases)
        file << purchase.serialize() << "\n";

    return true;
}

std::vector<Purchase> Storage::loadPurchases(const std::string& path)
{
    std::vector<Purchase> purchases;
    std::ifstream file(path);
    std::string line;

    if (!file.is_open())
        return purchases;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        try
        {
            purchases.push_back(Purchase::deserialize(line));
        }
        catch (const std::exception& e)
        {
            std::cerr << "Skipping invalid purchase line: " << e.what() << "\n";
        }
    }

    return purchases;
}

// ============================================================
// Sales
// ============================================================

bool Storage::saveSales(const std::vector<Sale>& sales, const std::string& path)
{
    std::ofstream file(path);
    if (!file.is_open())
        return false;

    for (const auto& sale : sales)
        file << sale.serialize() << "\n";

    return true;
}

std::vector<Sale> Storage::loadSales(const std::string& path)
{
    std::vector<Sale> sales;
    std::ifstream file(path);
    std::string line;

    if (!file.is_open())
        return sales;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        try
        {
            sales.push_back(Sale::deserialize(line));
        }
        catch (const std::exception& e)
        {
            std::cerr << "Skipping invalid sale line: " << e.what() << "\n";
        }
    }

    return sales;
}