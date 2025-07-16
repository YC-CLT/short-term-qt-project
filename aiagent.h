#ifndef AIAGENT_H
#define AIAGENT_H

#include "ui_mainwindow.h"
#include <QWidget>


#include <QNetworkAccessManager>
#include <QNetworkReply>


#include <QString>


#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QScrollBar>
#include <QTextBrowser>
#include <QTextDocument>
#include <QTextFormat>
#include <QTextDocumentFragment>
#include <QString>
#include <QTextCursor>
#include <QTextBlock>
#include <QRegularExpression>

#include <QDebug>

class AIAgent : public QWidget
{
    Q_OBJECT
public:
    explicit AIAgent(QWidget *parent = nullptr, Ui::MainWindow* mainUi = nullptr);
    void appendMessage(const QString &msg, bool isUser);
    void initMessageStyle();
    void updateLastMessage(const QString &chunk);

signals:
    void messageReceived(const QString& message);

public slots:
    void setTheme(bool theme);//设置ai的主题(默认暗黑)
    void sendMessage();  // 添加发送消息的槽函数

private:

    Ui::MainWindow* ui;
    QNetworkAccessManager* manager;
    QNetworkReply* reply;
    QJsonDocument jsonDoc;  // 保留JSON相关成员
    QJsonObject jsonObj;
};

#endif // AIAGENT_H
