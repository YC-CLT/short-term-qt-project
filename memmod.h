#ifndef MEMMOD_H
#define MEMMOD_H

#include "ui_mainwindow.h"

#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

#include <QMessageBox>
#include <QTableView>
#include <QSqlTableModel>

#include <QFile>
#include <QDir>
#include <QFileDialog>



class MemMod : public QWidget
{
    Q_OBJECT

public:
    explicit MemMod(QWidget *parent = nullptr, Ui::MainWindow* mainUi = nullptr);

    ~MemMod();

private:
    QSqlDatabase memDb;
    QSqlTableModel *model;
    Ui::MainWindow* ui;
};

#endif // MEMMOD_H
