#ifndef SETTINGMOD_H
#define SETTINGMOD_H

#include "ui_mainwindow.h"

#include <QWidget>

class SettingMod : public QWidget
{
    Q_OBJECT

public:
    explicit SettingMod(QWidget *parent = nullptr, Ui::MainWindow* ui = nullptr);
    ~SettingMod();

private:
    Ui::MainWindow* ui;
};

#endif // SETTINGMOD_H
