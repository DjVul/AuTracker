#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AuTracker.h"

class AuTracker : public QMainWindow
{
    Q_OBJECT

public:
    AuTracker(QWidget* parent = nullptr);
    ~AuTracker();

private slots:
    void onLoginClicked();
    void onSignUpClicked();

private:
    Ui::AuTrackerClass ui;
};