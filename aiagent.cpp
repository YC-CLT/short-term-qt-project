#include "aiagent.h"
#include "mainwindow.h"

AIAgent::AIAgent(QWidget *parent, Ui::MainWindow* mainUi)
    : QWidget{parent}, ui(mainUi)
{
    // 初始化网络管理器
    manager = new QNetworkAccessManager(this);
    
    // 连接发送按钮
    connect(ui->AISendButton, &QPushButton::clicked, this, &AIAgent::sendMessage);
    
    // 初始化消息样式
    initMessageStyle();

    connect(qobject_cast<MainWindow*>(parent), &MainWindow::themeChanged,this,&setTheme);
}

void AIAgent::sendMessage()
{
    QString message = ui->AITextEdit->toPlainText().trimmed();
    if(message.isEmpty()) return;

    // 添加用户消息
    appendMessage(message, true);
    ui->AITextEdit->clear();

    // 构造Ollama API请求
    QNetworkRequest request(QUrl("http://localhost:11434/api/generate"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QJsonObject json{
        {"model", "deepseek-r1:8b"},
        {"prompt", message},
        {"stream", false}  // 关闭流式传输
    };
    
    reply = manager->post(request, QJsonDocument(json).toJson());

    // 统一响应处理
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if(reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QString fullResponse = doc.object()["response"].toString();
            appendMessage(fullResponse, false);
        } else {
            appendMessage("请求失败：" + reply->errorString(), false);
        }
        reply->deleteLater();
    });
}


void AIAgent::appendMessage(const QString &msg, bool isUser)
{
    // 创建副本后替换换行符
    QString formattedMsg = msg;
    formattedMsg.replace('\n', "");  // 使用char类型参数
    
    QString html = QString("<div class='%1-msg'>%2</div>")
                   .arg(isUser ? "user" : "ai")
                   .arg(formattedMsg);

    ui->AITextBrowser->append(html);
    ui->AITextBrowser->verticalScrollBar()->setValue(
    ui->AITextBrowser->verticalScrollBar()->maximum()
    );
}

void AIAgent::initMessageStyle()
{
    // 默认使用深色模式
    ui->AITextBrowser->document()->setDefaultStyleSheet("");
    setTheme(true);
}

void AIAgent::setTheme(bool dark)
{
    if(dark) {
        QMessageBox::information(this, "提示", "深色模式已启用");
        // 深色模式样式 - 优化版
        QString styleSheet = 
            "div.user-msg {"
            "   background: linear-gradient(135deg, #1565C0 0%, #0D47A1 100%);"
            "   color: rgba(255,255,255,0.95);"
            "   border-radius: 18px 18px 0 18px;"
            "   padding: 14px 18px;"
            "   margin: 8px 0 8px 25%;"
            "   box-shadow: 0 4px 12px rgba(0,0,0,0.2);"
            "   transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);"
            "}"
            "div.ai-msg {"
            "   background: linear-gradient(135deg, #2c3e50 0%, #1a252f 100%);"
            "   color: rgba(236,240,241,0.95);"
            "   border-radius: 18px 18px 18px 0;"
            "   padding: 14px 18px;"
            "   margin: 8px 25% 8px 0;"
            "   box-shadow: 0 4px 12px rgba(0,0,0,0.15);"
            "   transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);"
            "}"
            "div.ai-msg::before, div.user-msg::before {"
            "   content: '';"
            "   position: absolute;"
            "   width: 0;"
            "   height: 0;"
            "   border: 8px solid transparent;"
            "   margin-top: 10px;"
            "}"
            "div.ai-msg::before {"
            "   border-right-color: #1a252f;"
            "   left: -16px;"
            "}"
            "div.user-msg::before {"
            "   border-left-color: #0D47A1;"
            "   right: -16px;"
            "}"
            "div.ai-msg, div.user-msg {"
            "   max-width: 75%;"
            "   font-size: 16px;"
            "   line-height: 1.7;"
            "   font-family: 'Segoe UI', 'PingFang SC', 'Microsoft YaHei', sans-serif;"
            "   position: relative;"
            "   word-wrap: break-word;"
            "}";
        ui->AITextBrowser->document()->setDefaultStyleSheet(styleSheet);
        ui->AITextBrowser->setStyleSheet(""); // 强制刷新样式
        
    } else {
        QMessageBox::information(this, "提示", "浅色模式已启用");
        // 浅色模式样式 - 优化版
        QString styleSheet =
            "div.user-msg {"
            "   background: linear-gradient(135deg, #3498db 0%, #2980b9 100%);"
            "   color: rgba(0, 0, 0, 0.95);"  // 保持用户消息文字为黑色
            "   border-radius: 18px 18px 0 18px;"
            "   padding: 14px 18px;"
            "   margin: 8px 0 8px 25%;"
            "   box-shadow: 0 4px 12px rgba(0,0,0,0.1);"
            "   transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);"
            "}"
            "div.ai-msg {"
            "   background: linear-gradient(135deg, #ecf0f1 0%, #dfe6e9 100%);"
            "   color: rgba(27, 38, 49, 0.95);"  // AI消息保持深色文字
            "   border-radius: 18px 18px 18px 0;"
            "   padding: 14px 18px;"
            "   margin: 8px 25% 8px 0;"
            "   box-shadow: 0 4px 12px rgba(0,0,0,0.05);"
            "   transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);"
            "}"
            "div.ai-msg::before, div.user-msg::before {"
            "   content: '';"
            "   position: absolute;"
            "   width: 0;"
            "   height: 0;"
            "   border: 8px solid transparent;"
            "   margin-top: 10px;"
            "}"
            "div.ai-msg::before {"
            "   border-right-color: #85888aff;"
            "   left: -16px;"
            "}"
            "div.user-msg::before {"
            "   border-left-color: #1e5d87ff;"
            "   right: -16px;"
            "}"
            "div.ai-msg, div.user-msg {"
            "   max-width: 75%;"
            "   font-size: 16px;"
            "   line-height: 1.7;"
            "   font-family: 'Segoe UI', 'PingFang SC', 'Microsoft YaHei', sans-serif;"
            "   position: relative;"
            "   word-wrap: break-word;"
            "}";
        ui->AITextBrowser->document()->setDefaultStyleSheet(styleSheet);
        ui->AITextBrowser->setStyleSheet(""); // 强制刷新样式
    }
    
    // 强制重新渲染所有内容
    ui->AITextBrowser->document()->setPlainText(ui->AITextBrowser->toPlainText());
}