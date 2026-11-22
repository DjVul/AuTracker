# AuTracker

A **desktop application for tracking physical gold purchases and portfolio performance**. Record gold purchases and sales, follow live and historical gold prices, and visualize profit over time in multiple currencies.

Built with **C++ and Qt 6 (Widgets)**. It uses **SHA-256** for password hashing, **libcurl + nlohmann/json** for API communication, and stores data locally in CSV files. The code is split into small modules, so it is easy to follow if you are learning C++/Qt.

---

## Features

- **User accounts** with optional password protection (SHA-256 hashing)
- **Purchase tracking**: grams, price per gram, currency, date
- **Sales tracking**: sell gold, calculate realized profit, delete sales
- **Portfolio overview**: total invested, current value, profit/loss
- **Live gold price** from the XAUS API, auto-refreshed every 5 minutes
- **Historical charts**: 1D, 7D, 30D, 60D, 6M, 1Y, 2Y, 5Y
- **Profit display**: switch between percentage (%) and absolute profit in the selected currency
- **Multi-currency support**: RSD, USD, EUR, GBP, JPY, CAD
- **Currency conversion** with live exchange rates from the Frankfurter API

---

## How It Works

### Architecture

```
┌─────────────────────────────────────────────────────┐
│ MainWindow (Qt UI)                                  │
│ ├── Portfolio tab → statistics + chart              │
│ ├── Purchases tab → add/delete purchases            │
│ ├── Sales tab     → sell/delete sales               │
│ └── Profile tab   → change password, logout         │
├─────────────────────────────────────────────────────┤
│ Models                                              │
│ ├── User     → username, password hash              │
│ ├── Purchase → grams, price, currency, date         │
│ └── Sale     → buy/sell price, profit               │
├─────────────────────────────────────────────────────┤
│ Services                                            │
│ ├── Storage       → CSV save/load                   │
│ └── PriceProvider → XAUS API + currency conversion  │
└─────────────────────────────────────────────────────┘
```

### Data Flow

1. **Login**: the user enters credentials, the password is hashed and compared.
2. **Purchases**: the user adds gold purchases, saved to `purchases.csv`.
3. **Sales**: the user sells gold, saved to `sales.csv`, and the purchase is removed.
4. **Portfolio**: purchases are aggregated, the live price is fetched, and profit is calculated.
5. **Charts**: historical gold prices are fetched from XAUS in the selected currency, and profit is calculated for each day.

### How the Chart Profit Is Calculated

- Each purchase's baseline is the gold price on its purchase date, so the chart reflects only the movement of the gold price and is independent of currency exchange rates.
- A purchase is included in the calculation only from its purchase date onward.
- Profit for a given day is `(current value - total invested) / total invested`, which is a weighted average over all purchases made up to that day.
- If a purchase date has no price (weekend, holiday), the last known price before it is used. If a purchase is older than the selected range, the first available price is used, so choose a range that covers your earliest purchase.

### Why CSV?

CSV is simple, human-readable, and easy to debug. For a local desktop app, it is sufficient. Each file has one record per line, with fields separated by commas.

---

## Requirements

- **Windows** (tested on Windows 10/11)
- **Visual Studio 2022** (or Build Tools)
- **Qt 6.12** (MSVC 2022 64-bit)
- **vcpkg** (for libcurl)

---

## Build Instructions

### 1. Clone the repository

```bash
git clone https://github.com/DjVul/AuTracker.git
cd AuTracker
```

### 2. Install Qt 6.12

1. Open the Qt Maintenance Tool.
2. Add **Qt 6.12.0 → MSVC 2022 64-bit**.
3. Add **Qt Charts** (deprecated, but required).

### 3. Install vcpkg and libcurl

```bash
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install curl:x64-windows
.\vcpkg integrate install
```

### 4. Open the solution

1. Open `AuTracker.sln` in Visual Studio 2022.
2. Go to **Extensions → Qt VS Tools → Qt Versions**.
3. Add `C:\Qt\6.12.0\msvc2022_64\bin\qmake.exe` and set it as active.

### 5. Build

1. Set the configuration to **Release** and the platform to **x64**.
2. Choose **Build → Build Solution**.

### 6. Run

The executable is in `x64\Release\AuTracker.exe`. Copy `libcurl.dll` from `C:\vcpkg\installed\x64-windows\bin\` next to the `.exe`.

---

## Usage

1. Launch the application.
2. Register a new user or log in with existing credentials.
3. Add purchases in the **Purchases** tab (grams, price, currency, date).
4. View statistics and the historical chart in the **Portfolio** tab. Use the dropdown to switch between percentage and absolute profit.
5. Record sales in the **Sales** tab to sell gold and see realized profit.
6. Manage your account in the **Profile** tab (change password, logout).

---

## Data Files

The application stores data in three CSV files, created automatically:

| File            | Format                                                      | Example                                                    |
|-----------------|-------------------------------------------------------------|------------------------------------------------------------|
| `users.csv`     | `username,passwordHash,enabled`                             | `marko,8d969eef...,1`                                      |
| `purchases.csv` | `owner,grams,price,currency,date`                           | `marko,10.0,65.5,EUR,15-01-2025`                           |
| `sales.csv`     | `owner,grams,buyPrice,sellPrice,currency,buyDate,sellDate`  | `marko,10.0,65.5,70.0,EUR,15-01-2025,20-06-2025`           |

> **Note:** These files are not committed to the repository (see `.gitignore`). They are created locally the first time you run the app.

---

## Dependencies

| Library        | Purpose           | License    |
|----------------|-------------------|------------|
| Qt 6           | GUI framework     | LGPL       |
| libcurl        | HTTP requests     | MIT-like   |
| nlohmann/json  | JSON parsing      | MIT        |
| PicoSHA2       | SHA-256 hashing   | MIT        |

---

## License

Apache License 2.0. See [LICENSE](LICENSE) for details.

## Notice

AuTracker  
Copyright (c) 2025 Ђорђе Вуловић / Djordje Vulovic

Contact: dj.vulovicc@gmail.com  
GitHub: https://github.com/DjVul