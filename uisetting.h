#ifndef UISETTING_H
#define UISETTING_H

#include "ui_mainwindow.h"
#include <QWidget>
#include <QPushButton>

class UiSetting : public QWidget
{
    Q_OBJECT
public:
    explicit UiSetting(QWidget *parent = nullptr, Ui::MainWindow* mainUi = nullptr);
private:
    Ui::MainWindow* ui;
};
#endif // UISETTING_H
