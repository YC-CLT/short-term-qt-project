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
#include <QPainter>

#include <QTableView>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QItemDelegate>
#include <QStyledItemDelegate>

#include <QFile>
#include <QDir>
#include <QFileDialog>




class MemMod : public QWidget
{
    Q_OBJECT

public:
    explicit MemMod(QWidget *parent = nullptr, Ui::MainWindow* mainUi = nullptr);

    ~MemMod();

    // 添加与重要日模块类似的方法
    void refreshMemTable();
    void commitMemTable();
    void deleteMemTable();
    void cancelMemTable();
    void insertMemTable();
private:
    QSqlDatabase memDb;
    QSqlTableModel *model;
    Ui::MainWindow* ui;

};

class CenterAlignDelegate : public QStyledItemDelegate {
public:
    explicit CenterAlignDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem opt = option;
        opt.displayAlignment = Qt::AlignCenter;
        QStyledItemDelegate::paint(painter, opt, index);
    }
};
#endif // MEMMOD_H
