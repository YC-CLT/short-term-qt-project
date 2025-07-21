#ifndef DAYMOD_H
#define DAYMOD_H

#include "ui_mainwindow.h"
#include <QWidget>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

#include <QDebug>
#include <QTimer>

#include <QThread>
#include <QMessageBox>

#include <QTableView>
#include <QSqlTableModel>

#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QStyledItemDelegate>

#include <QPainter>


class DayMod : public QWidget {
public:
    DayMod(QWidget* parent, Ui::MainWindow* mainUi);  // 修改构造函数
    ~DayMod();
    void refreshDayTable();
    void commitDayTable();
    void deleteDayTable();
    void cancelDayTable();
    void insertDayTable();
    void updateFirstEventMessage(); 
private:
    Ui::MainWindow* ui;  // 添加UI指针成员
    QSqlTableModel* model;  // 模型指针
    QSqlDatabase dayDb;  // 添加数据库成员变量声明
};

class BoolDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BoolDelegate(QObject* parent = nullptr, bool enableGrayEffect = false);
    
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    
private:
    bool m_enableGrayEffect;
};
#endif // DAYMOD_H