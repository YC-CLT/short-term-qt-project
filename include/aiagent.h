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
    // 修改函数签名，添加流式参数
    // 只在头文件声明中保留默认参数
    void appendMessage(const QString &msg, bool isUser, bool isStreaming = true);
    void initMessageStyle();
    void setSystemPrompt(const QString &prompt);  // 新增设置系统提示词的函数

signals:
    // 保持现有信号不变
    void messageReceived(const QString& message);

public slots:
    void clearChatHistory();  // 新增清除功能
    void setTheme(bool theme);
    void sendMessage();

private:
    // 已有成员变量保持不变
    Ui::MainWindow* ui;
    QNetworkAccessManager* manager;
    QNetworkReply* reply;
    QString systemPrompt;  // 系统提示词
    
    // 新增的成员变量已正确定义
    QList<QJsonObject> messageHistory;  
    QString currentResponse;            
};

#endif // AIAGENT_H
