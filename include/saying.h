#ifndef SAYING_H
#define SAYING_H
#include "ui_mainwindow.h"

#include <QWidget>
#include <QString>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QTimerEvent>
#include <QEvent>
#include <QRandomGenerator>

class Saying : public QWidget
{
    Q_OBJECT
public:
    explicit Saying(QWidget *parent = nullptr,Ui::MainWindow *ui = nullptr);    
    void typeNextChar();
    void backspaceText();

signals:

private:
    Ui::MainWindow *ui;

    QNetworkAccessManager *manager;
    QNetworkReply *reply;

    QJsonDocument jsonDoc;
    QJsonObject jsonObj;
    QJsonArray jsonArray;

    QString token;     
    QString m_typingContent;  // 当前正在输入的内容
    int m_charPos = 0;        // 当前字符位置

    bool m_isBackspacing = false; // 是否处于回退状态
    bool m_showCursor = true; // 光标显示状态

private slots:
    void replyFinished();
};

#endif // SAYING_H
