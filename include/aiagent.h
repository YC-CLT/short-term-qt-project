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
    void messageReceived(const QString& message);

public slots:
    void clearChatHistory();  // 清除聊天历史
    void setTheme(bool theme);
    void sendMessage();

private:
    Ui::MainWindow* ui;
    QNetworkAccessManager* manager;
    QNetworkReply* reply;
    QString systemPrompt;  // 系统提示词
    
    // 历史记录的json对象
    QList<QJsonObject> messageHistory; 
    // 最新的回复
    QString currentResponse;            
};

#endif // AIAGENT_H
