#ifndef PURCHASE_H
#define PURCHASE_H

#include <string>

class Purchase 
{
public:

	enum class Currency { RSD, USD, EUR, GBP, JPY, CAD };

	Purchase(const std::string& owner, double grams, double pricePerGram,
		Currency currency, const std::string& date = "");

	// Getters
	std::string getOwner() const;
	std::string getDate() const;
	double getGrams() const;
	double getPricePerGram() const;
	Currency getCurrency() const;

	// Setters
	void setOwner(const std::string& owner);
	void setDate(const std::string& date);
	void setGrams(double grams);
	void setPricePerGram(double pricePerGram);
	void setCurrency(Currency currency);

	// Helper functions
	static bool isValidCurrency(const Currency currency);
	static bool isValidDate(const std::string& date);

	double getTotalPrice() const;
	static std::string currencyToString(Currency currency);
	static Currency stringToCurrency(const std::string& currencyStr);
	void print() const;
	std::string serialize() const;
	static Purchase deserialize(const std::string& data);


private:

	std::string owner;
	std::string date;
	double grams;
	double pricePerGram;
	Currency currency;
};


#endif // PURCHASE_H

