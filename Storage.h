#ifndef STORAGE_H
#define STORAGE_H

#include "User.h"
#include "Purchase.h"
#include "Sale.h"

#include <vector>
#include <string>

class Storage
{
public:
    static bool saveUsers(const std::vector<User>& users, const std::string& path);
    static std::vector<User> loadUsers(const std::string& path);

    static bool savePurchases(const std::vector<Purchase>& purchases, const std::string& path);
    static std::vector<Purchase> loadPurchases(const std::string& path);

    static bool saveSales(const std::vector<Sale>& sales, const std::string& path);
    static std::vector<Sale> loadSales(const std::string& path);
};

#endif // STORAGE_H