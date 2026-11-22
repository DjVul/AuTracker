#pragma once

#include <QMainWindow>
#include <QTimer>
#include "ui_MainWindow.h"
#include "Purchase.h"
#include "Sale.h"

#include <QtCharts/QChartView>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString& username, QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onRefreshPriceClicked();
    void onCurrencyChanged(int index);
    void onAddPurchaseClicked();
    void onDeletePurchaseClicked();
    void onSellClicked();
    void onSalesCurrencyChanged(int index);
    void onChangePasswordClicked();
    void onRemovePasswordClicked();
    void onLogoutClicked();
    void onChartTabChanged(int index);
    void onChartDisplayModeChanged(int index);
    void onDeleteSaleClicked();

private:
    Ui::MainWindowClass* ui;
    QString currentUser;
    QTimer* refreshTimer;

    double cachedPrices[6];
    Purchase::Currency selectedCurrency;
    Sale::Currency selectedSalesCurrency;

    void updatePortfolio();
    void refreshPurchasesTable();
    void refreshSalesPurchaseCombo();
    void refreshSalesTable();
    void updateSalesSummary();
    Purchase::Currency currencyFromIndex(int index);
    int getSelectedPurchaseRow() const;
    void updateProfileTab();
    void updateChart(int daysBack, QChartView* chartView);
    void refreshCurrentChart();
};