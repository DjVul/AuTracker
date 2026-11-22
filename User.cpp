#include "User.h"
#include "picosha2.h"

#include <stdexcept>
#include <sstream>
#include <iostream>

// Constructor
User::User(const std::string& username,
    const std::string& passwordHash,
    bool passwordEnabled)
    : username(username),
    passwordHash(passwordHash),
    passwordEnabled(passwordEnabled)
{
    if (username.empty())
        throw std::invalid_argument("Username cannot be empty.");

    // If a hash is provided but enabled flag is false, enable it automatically
    if (!passwordHash.empty() && !passwordEnabled)
        this->passwordEnabled = true;

    // If no hash is provided, ensure password is disabled
    if (passwordHash.empty())
        this->passwordEnabled = false;
}

// Getters
std::string User::getUsername() const
{
    return username;
}

std::string User::getPasswordHash() const
{
    return passwordHash;
}

bool User::hasPassword() const
{
    return passwordEnabled;
}

// Password management
void User::setPassword(const std::string& plainPassword)
{
    if (plainPassword.empty())
        throw std::invalid_argument("Password cannot be empty.");

    std::string hash;
    picosha2::hash256_hex_string(plainPassword, hash);
    this->passwordHash = hash;
    this->passwordEnabled = true;
}

void User::removePassword()
{
    this->passwordHash.clear();
    this->passwordEnabled = false;
}

bool User::checkPassword(const std::string& plainPassword) const
{
    if (!passwordEnabled)
        return false;

    std::string inputHash;
    picosha2::hash256_hex_string(plainPassword, inputHash);
    return inputHash == this->passwordHash;
}

// Serialization
std::string User::serialize() const
{
    return username + "," +
        passwordHash + "," +
        (passwordEnabled ? "1" : "0");
}

User User::deserialize(const std::string& line)
{
    std::stringstream ss(line);
    std::string usernameStr, hashStr, enabledStr;

    std::getline(ss, usernameStr, ',');
    std::getline(ss, hashStr, ',');
    std::getline(ss, enabledStr, ',');

    // Validate that all three fields are present
    if (usernameStr.empty())
        throw std::invalid_argument("Invalid line: missing username.");
    if (enabledStr.empty())
        throw std::invalid_argument("Invalid line: missing password enabled flag.");

    // Validate enabled flag
    if (enabledStr != "0" && enabledStr != "1")
        throw std::invalid_argument("Invalid line: password enabled flag must be 0 or 1.");

    bool enabled = (enabledStr == "1");

    return User(usernameStr, hashStr, enabled);
}

// Print (without hash!)
void User::print() const
{
    std::cout << "Username: " << username
        << " (password: " << (passwordEnabled ? "enabled" : "disabled") << ")\n";
}