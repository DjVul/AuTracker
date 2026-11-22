#include "MainWindow.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <vector>
#include "Purchase.h"
#include "Storage.h"
#include "PriceProvider.h"
#include <iostream>


#include <QPainter>
#include <QDateTime>
//#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

//using namespace QtCharts;

MainWindow::MainWindow(const QString& username, QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindowClass)
    , currentUser(username)
    , selectedCurrency(Purchase::Currency::EUR)
    , selectedSalesCurrency(Sale::Currency::EUR)
{
    ui->setupUi(this);
    setWindowTitle("AuTracker — " + username);

    for (int i = 0; i < 6; ++i)
        cachedPrices[i] = 0.0;

    ui->portfolioCurrencyCombo->setCurrentIndex(2);
    ui->salesCurrencyCombo->setCurrentIndex(2);

    // Chart tabs connection
    connect(ui->portfolioChartTabs, &QTabWidget::currentChanged,
        this, &MainWindow::onChartTabChanged);

    // Portfolio connections
    connect(ui->portfolioRefreshButton, &QPushButton::clicked,
        this, &MainWindow::onRefreshPriceClicked);
    connect(ui->portfolioCurrencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onCurrencyChanged);

    // Purchases connections
    connect(ui->purchasesAddButton, &QPushButton::clicked,
        this, &MainWindow::onAddPurchaseClicked);
    connect(ui->purchasesDeleteButton, &QPushButton::clicked,
        this, &MainWindow::onDeletePurchaseClicked);

    // Sales connections
    connect(ui->salesSellButton, &QPushButton::clicked,
        this, &MainWindow::onSellClicked);
    connect(ui->salesCurrencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onSalesCurrencyChanged);

    // Setup purchases table columns
    ui->purchasesTable->setColumnCount(5);
    ui->purchasesTable->setHorizontalHeaderLabels(
        { "Date", "Grams", "Price/g", "Currency", "Total" });

    // Setup sales table columns
    ui->salesTable->setColumnCount(6);
    ui->salesTable->setHorizontalHeaderLabels(
        { "Sell Date", "Grams", "Buy Price", "Sell Price", "Profit", "Currency" });

    // Timer for auto-refresh
    refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::onRefreshPriceClicked);
    refreshTimer->start(5 * 60 * 1000);

    // Profile connections
    connect(ui->profileChangePasswordButton, &QPushButton::clicked,
        this, &MainWindow::onChangePasswordClicked);
    connect(ui->profileRemovePasswordButton, &QPushButton::clicked,
        this, &MainWindow::onRemovePasswordClicked);
    connect(ui->profileLogoutButton, &QPushButton::clicked,
        this, &MainWindow::onLogoutClicked);

    connect(ui->chartDisplayModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &MainWindow::onChartDisplayModeChanged);

    connect(ui->salesDeleteButton, &QPushButton::clicked,
        this, &MainWindow::onDeleteSaleClicked);

    // Refresh all views
    updatePortfolio();
    refreshPurchasesTable();
    refreshSalesPurchaseCombo();
    refreshSalesTable();
    updateSalesSummary();
    onRefreshPriceClicked();
    updateProfileTab();

    // Initial chart — 1 Day (index 0)
    ui->portfolioChartTabs->setCurrentIndex(1);
    ui->chartDisplayModeCombo->setCurrentIndex(0);   // default: %
    refreshCurrentChart();
}

MainWindow::~MainWindow()
{
    delete ui;
}

Purchase::Currency MainWindow::currencyFromIndex(int index)
{
    switch (index)
    {
    case 0: return Purchase::Currency::RSD;
    case 1: return Purchase::Currency::USD;
    case 2: return Purchase::Currency::EUR;
    case 3: return Purchase::Currency::GBP;
    case 4: return Purchase::Currency::JPY;
    case 5: return Purchase::Currency::CAD;
    default: return Purchase::Currency::EUR;
    }
}

void MainWindow::onCurrencyChanged(int index)
{
    selectedCurrency = currencyFromIndex(index);

    if (cachedPrices[static_cast<int>(selectedCurrency)] > 0)
    {
        double price = cachedPrices[static_cast<int>(selectedCurrency)];
        ui->portfolioGoldPriceLabel->setText(
            QString::number(price, 'f', 2) + " " +
            QString::fromStdString(Purchase::currencyToString(selectedCurrency)) + "/g");
        updatePortfolio();
    }
    else
    {
        onRefreshPriceClicked();
    }

    // Refresh the chart immediately so Y axis updates with new currency
    refreshCurrentChart();
}

void MainWindow::onRefreshPriceClicked()
{
    double price = PriceProvider::getCurrentPricePerGram(selectedCurrency);

    if (price > 0)
    {
        cachedPrices[static_cast<int>(selectedCurrency)] = price;
        ui->portfolioGoldPriceLabel->setText(
            QString::number(price, 'f', 2) + " " +
            QString::fromStdString(Purchase::currencyToString(selectedCurrency)) + "/g");
    }
    else
    {
        ui->portfolioGoldPriceLabel->setText("Failed to fetch price");
    }

    updatePortfolio();
}

void MainWindow::updatePortfolio()
{
    std::vector<Purchase> allPurchases = Storage::loadPurchases("purchases.csv");

    double totalGrams = 0.0;
	double totalInvestedEur = 0.0;   // All calculations are done in EUR for consistency

	// Summarize current user's purchases
    for (const auto& p : allPurchases)
    {
        if (p.getOwner() == currentUser.toStdString())
        {
            totalGrams += p.getGrams();

			// Convert to EUR for consistent portfolio value calculation
            double costInOriginal = p.getTotalPrice();
            double costInEur = PriceProvider::convertCurrency(
                costInOriginal,
                p.getCurrency(),
                Purchase::Currency::EUR);

            totalInvestedEur += costInEur;
        }
    }

	// Show total grams in portfolio
    ui->portfolioGramsLabel->setText(QString::number(totalGrams, 'f', 2) + " g");

	// Convert total invested to selected currency for display
    double totalInvestedDisplay = PriceProvider::convertCurrency(
        totalInvestedEur,
        Purchase::Currency::EUR,
        selectedCurrency);

    QString currencyStr = QString::fromStdString(
        Purchase::currencyToString(selectedCurrency));

    ui->portfolioInvestedLabel->setText(
        QString::number(totalInvestedDisplay, 'f', 2) + " " + currencyStr);

	// Calculate current value and profit
    double currentPrice = cachedPrices[static_cast<int>(selectedCurrency)];

    if (currentPrice > 0 && totalGrams > 0)
    {
        double currentValue = totalGrams * currentPrice;
        double profit = currentValue - totalInvestedDisplay;
        double profitPercent = (totalInvestedDisplay > 0)
            ? (profit / totalInvestedDisplay) * 100.0
            : 0.0;

        ui->portfolioValueLabel->setText(
            QString::number(currentValue, 'f', 2) + " " + currencyStr);
        ui->portfolioProfitLabel->setText(
            QString::number(profit, 'f', 2) + " " + currencyStr +
            " (" + QString::number(profitPercent, 'f', 2) + "%)");
    }
    else
    {
        ui->portfolioValueLabel->setText("—");
        ui->portfolioProfitLabel->setText("Click 'Refresh Price'");
    }
}


void MainWindow::refreshPurchasesTable()
{
    std::vector<Purchase> allPurchases = Storage::loadPurchases("purchases.csv");

    // Filter only current user's purchases
    std::vector<Purchase> myPurchases;
    for (const auto& p : allPurchases)
    {
        if (p.getOwner() == currentUser.toStdString())
            myPurchases.push_back(p);
    }

    ui->purchasesTable->setRowCount(static_cast<int>(myPurchases.size()));

    for (size_t i = 0; i < myPurchases.size(); ++i)
    {
        const auto& p = myPurchases[i];

        ui->purchasesTable->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(p.getDate())));
        ui->purchasesTable->setItem(i, 1, new QTableWidgetItem(
            QString::number(p.getGrams(), 'f', 2)));
        ui->purchasesTable->setItem(i, 2, new QTableWidgetItem(
            QString::number(p.getPricePerGram(), 'f', 2)));
        ui->purchasesTable->setItem(i, 3, new QTableWidgetItem(
            QString::fromStdString(Purchase::currencyToString(p.getCurrency()))));
        ui->purchasesTable->setItem(i, 4, new QTableWidgetItem(
            QString::number(p.getTotalPrice(), 'f', 2)));
    }
}

void MainWindow::onAddPurchaseClicked()
{
    try
    {
        double grams = ui->purchasesGramsInput->value();
        double pricePerGram = ui->purchasesPriceInput->value();
        int currencyIndex = ui->purchasesCurrencyCombo->currentIndex();
        Purchase::Currency currency = currencyFromIndex(currencyIndex);
        QString date = ui->purchasesDateInput->text();

        Purchase p(currentUser.toStdString(), grams, pricePerGram,
            currency, date.toStdString());

        std::vector<Purchase> allPurchases = Storage::loadPurchases("purchases.csv");
        allPurchases.push_back(p);
        Storage::savePurchases(allPurchases, "purchases.csv");

        refreshPurchasesTable();
        refreshSalesPurchaseCombo();
        updatePortfolio();
        refreshCurrentChart();

        QMessageBox::information(this, "Success", "Purchase added.");
    }
    catch (const std::invalid_argument& e)
    {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}

int MainWindow::getSelectedPurchaseRow() const
{
    QList<QTableWidgetItem*> selected = ui->purchasesTable->selectedItems();
    if (selected.isEmpty())
        return -1;
    return selected.first()->row();
}

void MainWindow::onDeletePurchaseClicked()
{
    int row = getSelectedPurchaseRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Error", "Select a purchase to delete.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        "Are you sure you want to delete this purchase?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    // Load all purchases
    std::vector<Purchase> allPurchases = Storage::loadPurchases("purchases.csv");

    // Filter current user's purchases (same order as table)
    std::vector<Purchase> myPurchases;
    for (const auto& p : allPurchases)
    {
        if (p.getOwner() == currentUser.toStdString())
            myPurchases.push_back(p);
    }

    if (row >= static_cast<int>(myPurchases.size()))
        return;

    // Remove selected purchase
    myPurchases.erase(myPurchases.begin() + row);

    // Rebuild: other users' purchases + my remaining purchases
    std::vector<Purchase> newAll;
    for (const auto& p : allPurchases)
    {
        if (p.getOwner() != currentUser.toStdString())
            newAll.push_back(p);
    }
    for (const auto& p : myPurchases)
        newAll.push_back(p);

    Storage::savePurchases(newAll, "purchases.csv");

    refreshPurchasesTable();
    refreshSalesPurchaseCombo();
    updatePortfolio();
    refreshCurrentChart();

    QMessageBox::information(this, "Success", "Purchase deleted.");
}


void MainWindow::refreshSalesPurchaseCombo()
{
    ui->salesPurchaseCombo->clear();

    std::vector<Purchase> allPurchases = Storage::loadPurchases("purchases.csv");

    for (const auto& p : allPurchases)
    {
        if (p.getOwner() == currentUser.toStdString())
        {
            QString text = QString::number(p.getGrams(), 'f', 2) + " g " +
                QString::fromStdString(Purchase::currencyToString(p.getCurrency())) +
                " @ " + QString::number(p.getPricePerGram(), 'f', 2) +
                " (" + QString::fromStdString(p.getDate()) + ")";
            ui->salesPurchaseCombo->addItem(text);
        }
    }
}


void MainWindow::onSellClicked()
{
    int index = ui->salesPurchaseCombo->currentIndex();
    if (index < 0)
    {
        QMessageBox::warning(this, "Error", "No purchase selected.");
        return;
    }

    // Load all purchases and filter for current user
    std::vector<Purchase> allPurchases = Storage::loadPurchases("purchases.csv");
    std::vector<Purchase> myPurchases;
    for (const auto& p : allPurchases)
    {
        if (p.getOwner() == currentUser.toStdString())
            myPurchases.push_back(p);
    }

    if (index >= static_cast<int>(myPurchases.size()))
        return;

    Purchase selected = myPurchases[index];

    try
    {
        double sellPrice = ui->salesPriceInput->value();
        QString sellDate = ui->salesDateInput->text();

        // Create Sale object
        Sale sale(currentUser.toStdString(),
            selected.getGrams(),
            selected.getPricePerGram(),
            sellPrice,
			static_cast<Sale::Currency>(selected.getCurrency()),   // ← existing currency mapping
            selected.getDate(),
            sellDate.toStdString());

        // Save sale to sales.csv
        std::vector<Sale> allSales = Storage::loadSales("sales.csv");
        allSales.push_back(sale);
        Storage::saveSales(allSales, "sales.csv");

        // Delete only the first matching purchase from purchases.csv
        bool deleted = false;
        std::vector<Purchase> newAll;
        for (const auto& p : allPurchases)
        {
            if (!deleted &&
                p.getOwner() == currentUser.toStdString() &&
                p.getDate() == selected.getDate() &&
                p.getGrams() == selected.getGrams() &&
                p.getPricePerGram() == selected.getPricePerGram() &&
                p.getCurrency() == selected.getCurrency())
            {
                deleted = true;
                continue;
            }
            newAll.push_back(p);
        }
        Storage::savePurchases(newAll, "purchases.csv");

        // Refresh all views
        refreshSalesPurchaseCombo();
        refreshSalesTable();
        updateSalesSummary();
        refreshPurchasesTable();
        updatePortfolio();

        QMessageBox::information(this, "Success", "Gold sold successfully.");
    }
    catch (const std::invalid_argument& e)
    {
        QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    }
}



void MainWindow::refreshSalesTable()
{
    std::vector<Sale> allSales = Storage::loadSales("sales.csv");

    std::vector<Sale> mySales;
    for (const auto& s : allSales)
    {
        if (s.getOwner() == currentUser.toStdString())
            mySales.push_back(s);
    }

    ui->salesTable->setRowCount(static_cast<int>(mySales.size()));

    for (size_t i = 0; i < mySales.size(); ++i)
    {
        const auto& s = mySales[i];
        QString currStr = QString::fromStdString(Sale::currencyToString(s.getCurrency()));

        ui->salesTable->setItem(i, 0, new QTableWidgetItem(
            QString::fromStdString(s.getSellDate())));
        ui->salesTable->setItem(i, 1, new QTableWidgetItem(
            QString::number(s.getGrams(), 'f', 2)));
        ui->salesTable->setItem(i, 2, new QTableWidgetItem(
            QString::number(s.getBuyPricePerGram(), 'f', 2)));
        ui->salesTable->setItem(i, 3, new QTableWidgetItem(
            QString::number(s.getSellPricePerGram(), 'f', 2)));
        ui->salesTable->setItem(i, 4, new QTableWidgetItem(
            QString::number(s.getProfit(), 'f', 2)));
        ui->salesTable->setItem(i, 5, new QTableWidgetItem(currStr));
    }
}


void MainWindow::updateSalesSummary()
{
    std::vector<Sale> allSales = Storage::loadSales("sales.csv");

    double totalGrams = 0.0;
    double totalRevenueEur = 0.0;
    double totalProfitEur = 0.0;

    for (const auto& s : allSales)
    {
        if (s.getOwner() != currentUser.toStdString()) continue;

        totalGrams += s.getGrams();

        // Convert Sale::Currency to Purchase::Currency for convertCurrency
        Purchase::Currency purchaseCurrency =
            static_cast<Purchase::Currency>(s.getCurrency());

        double revenueEur = PriceProvider::convertCurrency(
            s.getSellTotal(), purchaseCurrency, Purchase::Currency::EUR);
        double profitEur = PriceProvider::convertCurrency(
            s.getProfit(), purchaseCurrency, Purchase::Currency::EUR);

        totalRevenueEur += revenueEur;
        totalProfitEur += profitEur;
    }

    // Convert to selected currency for display
    Purchase::Currency selectedPurchaseCurrency =
        static_cast<Purchase::Currency>(selectedSalesCurrency);

    double totalRevenueDisplay = PriceProvider::convertCurrency(
        totalRevenueEur, Purchase::Currency::EUR, selectedPurchaseCurrency);
    double totalProfitDisplay = PriceProvider::convertCurrency(
        totalProfitEur, Purchase::Currency::EUR, selectedPurchaseCurrency);

    QString currStr = QString::fromStdString(
        Sale::currencyToString(selectedSalesCurrency));

    ui->salesTotalGramsLabel->setText(QString::number(totalGrams, 'f', 2) + " g");
    ui->salesTotalRevenueLabel->setText(
        QString::number(totalRevenueDisplay, 'f', 2) + " " + currStr);
    ui->salesRealizedProfitLabel->setText(
        QString::number(totalProfitDisplay, 'f', 2) + " " + currStr);
}

void MainWindow::onSalesCurrencyChanged(int index)
{
    switch (index)
    {
    case 0: selectedSalesCurrency = Sale::Currency::RSD; break;
    case 1: selectedSalesCurrency = Sale::Currency::USD; break;
    case 2: selectedSalesCurrency = Sale::Currency::EUR; break;
    case 3: selectedSalesCurrency = Sale::Currency::GBP; break;
    case 4: selectedSalesCurrency = Sale::Currency::JPY; break;
    case 5: selectedSalesCurrency = Sale::Currency::CAD; break;
    default: selectedSalesCurrency = Sale::Currency::EUR; break;
    }

    updateSalesSummary();
}


void MainWindow::updateProfileTab()
{
    // Show username
    ui->profileUsernameLabel->setText("Logged in as: " + currentUser);

    // Load users to find current user's password status
    std::vector<User> users = Storage::loadUsers("users.csv");

    for (const auto& u : users)
    {
        if (u.getUsername() == currentUser.toStdString())
        {
            if (u.hasPassword())
            {
                ui->profilePasswordStatusLabel->setText("Password: enabled");
                ui->profileRemovePasswordButton->setEnabled(true);
            }
            else
            {
                ui->profilePasswordStatusLabel->setText("Password: disabled");
                ui->profileRemovePasswordButton->setEnabled(false);
            }
            break;
        }
    }
}

void MainWindow::onChangePasswordClicked()
{
    QString newPass = ui->profileNewPasswordInput->text();
    QString confirmPass = ui->profileConfirmPasswordInput->text();

    if (newPass.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Password cannot be empty.");
        return;
    }

    if (newPass != confirmPass)
    {
        QMessageBox::warning(this, "Error", "Passwords do not match.");
        return;
    }

    // Load users
    std::vector<User> users = Storage::loadUsers("users.csv");

    bool found = false;
    for (auto& u : users)
    {
        if (u.getUsername() == currentUser.toStdString())
        {
            u.setPassword(newPass.toStdString());
            found = true;
            break;
        }
    }

    if (!found)
    {
        QMessageBox::warning(this, "Error", "User not found.");
        return;
    }

    Storage::saveUsers(users, "users.csv");

    // Clear inputs
    ui->profileNewPasswordInput->clear();
    ui->profileConfirmPasswordInput->clear();

    updateProfileTab();

    QMessageBox::information(this, "Success", "Password changed.");
}

void MainWindow::onRemovePasswordClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm",
        "Are you sure you want to remove your password?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    std::vector<User> users = Storage::loadUsers("users.csv");

    for (auto& u : users)
    {
        if (u.getUsername() == currentUser.toStdString())
        {
            u.removePassword();
            break;
        }
    }

    Storage::saveUsers(users, "users.csv");

    updateProfileTab();

    QMessageBox::information(this, "Success", "Password removed.");
}

void MainWindow::onLogoutClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm",
        "Are you sure you want to logout?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    // Stop the timer
    refreshTimer->stop();

    // Close MainWindow — this will end the application
    // (login window was already closed)
    this->close();
}

void MainWindow::updateChart(int daysBack, QChartView* chartView)
{
    if (!chartView) return;

    const Purchase::Currency displayCurrency = selectedCurrency;
    const bool showPercent = (ui->chartDisplayModeCombo->currentIndex() == 0);

    // Parsiranje datuma u više formata
    auto parseDate = [](const std::string& s) -> QDate {
        if (s.empty()) return QDate();
        const QString qs = QString::fromStdString(s);
        for (const char* fmt : { "yyyy-MM-dd", "d-M-yyyy", "dd-MM-yyyy" })
        {
            QDate d = QDate::fromString(qs, fmt);
            if (d.isValid()) return d;
        }
        return QDate();
        };

    // 1. Istorijske cene (po gramu, u izabranoj valuti)
    auto history = PriceProvider::getHistoricalPrices(displayCurrency, daysBack);
    if (history.empty()) return;

    QMap<QDate, double> prices;
    for (const auto& [dateStr, price] : history)
    {
        QDate d = QDate::fromString(QString::fromStdString(dateStr), "yyyy-MM-dd");
        if (d.isValid()) prices.insert(d, price);
    }
    if (prices.isEmpty()) return;

    auto priceAt = [&prices](const QDate& date) -> double {
        auto it = prices.upperBound(date);
        if (it == prices.begin()) return it.value();
        --it;
        return it.value();
        };

    // 2. Kupovine korisnika, konvertovane u displayCurrency
    struct Lot { QDate date; double grams; double buyPrice; };
    std::vector<Lot> lots;

    for (const auto& p : Storage::loadPurchases("purchases.csv"))
    {
        if (p.getOwner() != currentUser.toStdString()) continue;

        QDate d = parseDate(p.getDate());
        if (!d.isValid() || p.getGrams() <= 0) continue;

        double buyPriceDisplay = PriceProvider::convertCurrency(
            p.getPricePerGram(),
            p.getCurrency(),
            displayCurrency);

        lots.push_back({ d, p.getGrams(), buyPriceDisplay });
    }
    std::sort(lots.begin(), lots.end(),
        [](const Lot& a, const Lot& b) { return a.date < b.date; });

    // 3. Početak grafika
    QDate startDate = lots.empty() ? prices.firstKey() : lots.front().date;

    // 4. Za svaki dan: vrednost ili procenat
    QLineSeries* series = new QLineSeries();
    QPen goldPen(QColor(212, 175, 55));
    goldPen.setWidth(2);
    series->setPen(goldPen);

    QDateTime firstDt, lastDt;
    double minY = 0.0, maxY = 0.0;
    bool firstPoint = true;

    double totalGrams = 0.0;
    double totalInvested = 0.0;
    size_t nextLot = 0;

    for (auto it = prices.lowerBound(startDate); it != prices.end(); ++it)
    {
        const QDate day = it.key();
        const double currentPrice = it.value();

        while (nextLot < lots.size() && lots[nextLot].date <= day)
        {
            totalGrams += lots[nextLot].grams;
            totalInvested += lots[nextLot].grams * lots[nextLot].buyPrice;
            ++nextLot;
        }

        double yValue = 0.0;
        if (showPercent)
        {
            if (totalInvested > 0.0)
            {
                double currentValue = totalGrams * currentPrice;
                yValue = (currentValue - totalInvested) / totalInvested * 100.0;
            }
        }
        else
        {
            // Absolute profit in display currency
            double currentValue = totalGrams * currentPrice;
            yValue = currentValue - totalInvested;
        }

        if (firstPoint)
        {
            minY = maxY = yValue;
            firstDt = day.startOfDay();
            firstPoint = false;
        }
        else
        {
            minY = std::min(minY, yValue);
            maxY = std::max(maxY, yValue);
        }
        lastDt = day.startOfDay();
        series->append(day.startOfDay().toMSecsSinceEpoch(), yValue);
    }

    if (series->count() == 0) { delete series; return; }

    if (series->count() == 1 && firstDt.isValid())
    {
        lastDt = firstDt.addDays(1);
        series->append(lastDt.toMSecsSinceEpoch(), series->at(0).y());
    }

    // 5. Chart
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("");
    chart->legend()->hide();
    chart->setMargins(QMargins(5, 5, 5, 5));
    chart->setBackgroundRoundness(0);

    // 6. Ose
    QDateTimeAxis* axisX = new QDateTimeAxis();
    axisX->setTickCount(5);
    axisX->setFormat("dd-MM-yy");
    axisX->setGridLineVisible(false);
    axisX->setRange(firstDt, lastDt);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTickCount(5);
    axisY->setGridLineVisible(false);

    if (showPercent)
        axisY->setTitleText("Profit (%)");
    else
        axisY->setTitleText("Profit (" +
            QString::fromStdString(Purchase::currencyToString(displayCurrency)) + ")");

    axisY->setLabelFormat(showPercent ? "%.2f" : "%.2f");

    if (qFuzzyCompare(minY + 1.0, maxY + 1.0))
        axisY->setRange(minY - 1, maxY + 1);
    else
    {
        const double range = maxY - minY;
        axisY->setRange(minY - range * 0.05, maxY + range * 0.05);
    }
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 7. Postavi na view
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
}
void MainWindow::onChartTabChanged(int index)
{
    int daysBack = 0;
    QChartView* view = nullptr;

    switch (index)
    {
    case 0: daysBack = 1;    view = ui->chartView1D;  break;
    case 1: daysBack = 7;    view = ui->chartView7D;  break;
    case 2: daysBack = 30;   view = ui->chartView30D; break;
    case 3: daysBack = 60;   view = ui->chartView60D; break;
    case 4: daysBack = 180;  view = ui->chartView6M;  break;
    case 5: daysBack = 365;  view = ui->chartView1Y;  break;
    case 6: daysBack = 730;  view = ui->chartView2Y;  break;
    case 7: daysBack = 1825; view = ui->chartView5Y;  break;
    default: return;
    }

    if (!view) return;
    updateChart(daysBack, view);
}


void MainWindow::refreshCurrentChart()
{
    int index = ui->portfolioChartTabs->currentIndex();
    onChartTabChanged(index);
}

void MainWindow::onChartDisplayModeChanged(int index)
{
    refreshCurrentChart();
}

void MainWindow::onDeleteSaleClicked()
{
    // Get selected row from sales table
    QList<QTableWidgetItem*> selected = ui->salesTable->selectedItems();
    if (selected.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Select a sale to delete.");
        return;
    }

    int row = selected.first()->row();
    if (row < 0)
    {
        QMessageBox::warning(this, "Error", "Select a sale to delete.");
        return;
    }

    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Delete",
        "Are you sure you want to delete this sale?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes)
        return;

    // Load all sales
    std::vector<Sale> allSales = Storage::loadSales("sales.csv");

    // Filter current user's sales (same order as table)
    std::vector<Sale> mySales;
    for (const auto& s : allSales)
    {
        if (s.getOwner() == currentUser.toStdString())
            mySales.push_back(s);
    }

    if (row >= static_cast<int>(mySales.size()))
        return;

    // Remove selected sale
    mySales.erase(mySales.begin() + row);

    // Rebuild: other users' sales + my remaining sales
    std::vector<Sale> newAll;
    for (const auto& s : allSales)
    {
        if (s.getOwner() != currentUser.toStdString())
            newAll.push_back(s);
    }
    for (const auto& s : mySales)
        newAll.push_back(s);

    Storage::saveSales(newAll, "sales.csv");

    // Refresh views
    refreshSalesTable();
    updateSalesSummary();
    updatePortfolio();
    refreshCurrentChart();

    QMessageBox::information(this, "Success", "Sale deleted.");
}