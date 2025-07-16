#ifndef SETTINGMOD_H
#define SETTINGMOD_H

#include "ui_mainwindow.h"

#include "daymod.h"
#include "memmod.h"

#include <QWidget>
#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QStyledItemDelegate>
#include <QRandomGenerator>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QStandardPaths>


class SettingMod : public QWidget
{
    Q_OBJECT

public:
    explicit SettingMod(QWidget *parent = nullptr, Ui::MainWindow* ui = nullptr);
    ~SettingMod();

private slots:
    void showDayLog();  // 显示重要日日志
    void showMemLog();  // 显示备忘录日志
    void setBackground(int index);  // 背景设置
    void applyBackground(const QString& bgPath);  // 应用背景设置
    void clearPicCache();  // 清除图片缓存

private:
    Ui::MainWindow* ui;
    
    QDialog* dayLogDialog;  // 重要日日志显示对话框
    QDialog* memLogDialog;  // 备忘录日志显示对话框
    QSqlTableModel* dayModel;
    QSqlTableModel* memModel;
};
#endif // SETTINGMOD_H