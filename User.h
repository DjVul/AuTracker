#ifndef USER_H
#define USER_H

#include <string>

class User
{
public:
    // Constructor with default values
    User(const std::string& username,
        const std::string& passwordHash = "",
        bool passwordEnabled = false);

    // Getters
    std::string getUsername() const;
    std::string getPasswordHash() const;
    bool hasPassword() const;

    // Password management
    void setPassword(const std::string& plainPassword);
    void removePassword();
    bool checkPassword(const std::string& plainPassword) const;

    // Serialization
    std::string serialize() const;
    static User deserialize(const std::string& line);

    // Print (without hash!)
    void print() const;

private:
    std::string username;
    std::string passwordHash;
    bool passwordEnabled;
};

#endif // USER_H