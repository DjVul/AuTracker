#include "AuTracker.h"
#include <QMessageBox>
#include <vector>
#include "User.h"
#include "Storage.h"
#include "MainWindow.h"

AuTracker::AuTracker(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Poveži dugmad sa slotovima
    connect(ui.loginButton, &QPushButton::clicked, this, &AuTracker::onLoginClicked);
    connect(ui.signUpButton, &QPushButton::clicked, this, &AuTracker::onSignUpClicked);
}

AuTracker::~AuTracker()
{
}

void AuTracker::onLoginClicked()
{
    QString username = ui.usernameInput->text();
    QString password = ui.passwordInput->text();

    if (username.isEmpty())
    {
        ui.errorLabel->setText("Username cannot be empty.");
        return;
    }

    std::vector<User> users = Storage::loadUsers("users.csv");

    User* found = nullptr;
    for (auto& u : users)
    {
        if (u.getUsername() == username.toStdString())
        {
            found = &u;
            break;
        }
    }

    if (found == nullptr)
    {
        ui.errorLabel->setText("User not found.");
        return;
    }

    if (found->hasPassword())
    {
        if (!found->checkPassword(password.toStdString()))
        {
            ui.errorLabel->setText("Wrong password.");
            return;
        }
    }

    // Uspešno logovanje — otvori glavni prozor kao samostalan prozor
    MainWindow* mainWindow = new MainWindow(username, nullptr);   // nullptr — bez parent-a
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    mainWindow->show();

    // Trajno zatvori login prozor
    this->close();
}

void AuTracker::onSignUpClicked()
{
    QString username = ui.usernameInput->text();
    QString password = ui.passwordInput->text();

    if (username.isEmpty())
    {
        ui.errorLabel->setText("Username cannot be empty.");
        return;
    }

    std::vector<User> users = Storage::loadUsers("users.csv");

    // Proveri da li korisnik već postoji
    for (const auto& u : users)
    {
        if (u.getUsername() == username.toStdString())
        {
            ui.errorLabel->setText("User already exists.");
            return;
        }
    }

    // Kreiraj novog korisnika
    User newUser(username.toStdString());
    if (!password.isEmpty())
        newUser.setPassword(password.toStdString());

    users.push_back(newUser);
    Storage::saveUsers(users, "users.csv");

    // Uspešna registracija — otvori glavni prozor kao samostalan prozor
    ui.errorLabel->setText("");
    MainWindow* mainWindow = new MainWindow(username, nullptr);   // nullptr — bez parent-a
    mainWindow->setAttribute(Qt::WA_DeleteOnClose);
    mainWindow->show();

    // Trajno zatvori login prozor
    this->close();
}