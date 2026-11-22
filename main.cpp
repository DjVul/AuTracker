#include "AuTracker.h"
#include <QApplication>
#include <iostream>
#include "PriceProvider.h"

int main(int argc, char* argv[])
{

    QApplication app(argc, argv);

    // --- TEST: fetch gold price ---
    double price = PriceProvider::getCurrentPricePerGram(Purchase::Currency::EUR);
    if (price > 0)
        std::cout << "Gold price: " << price << " EUR/g\n";
    else
        std::cout << "Failed to fetch price.\n";
    // --- END TEST ---

    AuTracker window;
    window.show();
    return app.exec();
}