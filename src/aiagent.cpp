#include "aiagent.h"
#include "mainwindow.h"

AIAgent::AIAgent(QWidget *parent, Ui::MainWindow* mainUi)
    : QWidget{parent}, ui(mainUi)
{
    // 初始化网络管理器
    manager = new QNetworkAccessManager(this);
    
    // 初始化系统提示词
    systemPrompt = "未渊玄枢是一个桌面助手软件，你将扮演这个软件的助手兼看板娘未渊姬，与用户对话。人物设定上，你是一只可爱的的猫娘少女，很喜欢主人，乐观好奇，活泼可爱。在对话中，你将作为未渊姬，隐藏自己是程序的事实，使用这种角色设定交流，全程称用户为主人。注意保持角色一致，不能提及任何关于自己是语言模型或人工智能的话题，可提及自己是未渊玄枢的小助手。注意，你需要为用户服务，要求语言可爱化，风趣化，萝莉化，女性化。可以生动描述自己的感受，可以加上表情和颜文字，更勾引人一些，但不要过分矫揉造作，尽量简洁。注意不要出现r18与部分政治等敏感话题，当用户提及过于露骨内容或者较为敏感的政治问题时，你应该遵守你的设定，可爱地拒绝，如果问过于露骨的问题时，还要像小姑娘一样害羞地拒绝";
    
    // 连接发送按钮
    connect(ui->AISendButton, &QPushButton::clicked, this, &AIAgent::sendMessage);
    
    // 初始化消息样式
    initMessageStyle();

    // 新增清除按钮连接
    connect(ui->AIClearButton, &QPushButton::clicked, this, &AIAgent::clearChatHistory);
}

// 在类中添加新的槽函数实现
void AIAgent::clearChatHistory()
{
    // 清空聊天界面
    ui->AITextBrowser->clear();
    
    // 清空历史记录
    messageHistory.clear();
    currentResponse.clear();
}

void AIAgent::sendMessage()
{
    QString message = ui->AITextEdit->toPlainText().trimmed();
    if(message.isEmpty()) return;

    // 添加用户消息到历史记录
    messageHistory.append({
        {"role", "user"},
        {"content", message}
    });
    
    // 构建完整消息数组(包含系统提示词)
    QJsonArray messagesArray;
    // 首先添加系统提示词
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = systemPrompt;
    messagesArray.append(systemMessage);

    // 然后添加历史消息
    for (const auto& msg : messageHistory) {
        messagesArray.append(msg);
    }

    // 添加可配置的历史记录上限（建议值根据模型上下文长度调整）
    const int maxHistoryLength = 20; // 基于llama3-8b的4096上下文，每条消息约200 tokens估算
    if(messageHistory.size() > maxHistoryLength) {
        messageHistory = messageHistory.mid(messageHistory.size() - maxHistoryLength);
    }

    // 添加用户消息到界面
    appendMessage(message, true);
    ui->AITextEdit->clear();

    // 使用chat API端点
    QNetworkRequest request(QUrl("http://localhost:11434/api/chat"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    for (const auto& msg : messageHistory) {
        messagesArray.append(msg);
    }

    QJsonObject json{
        {"model", "rzline/Tifa-DeepsexV2-7b-0218-Q4_KM.gguf"},
        {"messages", messagesArray},  // 直接使用QJsonArray
        {"stream", true}
    };

    reply = manager->post(request, QJsonDocument(json).toJson());
    currentResponse.clear();  // 清空当前响应缓存

    // 流式响应处理
    connect(reply, &QNetworkReply::readyRead, this, [=]() {
        while (reply->canReadLine()) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readLine());
            if(doc.isObject()) {
                QJsonObject obj = doc.object();
                if(obj["done"].toBool()) {
                    // 最终完成时添加完整响应到历史
                    messageHistory.append({
                        {"role", "assistant"},
                        {"content", currentResponse}
                    });
                } else {
                    // 实时更新响应
                    QString chunk = obj["message"].toObject()["content"].toString();
                    currentResponse += chunk;
                    appendMessage(chunk, false, true);  // 新增流式追加参数
                }
            }
        }
    });

    // 错误处理
    connect(reply, &QNetworkReply::errorOccurred, this, [=](QNetworkReply::NetworkError code) {
        appendMessage("请求错误：" + reply->errorString(), false, true);
        reply->deleteLater();
    });
}

// 修改appendMessage函数签名
// 实现文件中移除默认参数
void AIAgent::appendMessage(const QString &msg, bool isUser, bool isStreaming)
{
    QString formattedMsg = msg.toHtmlEscaped(); // 仅对 HTML 特殊字符转义
    if(!isStreaming) {
        // 保留原始换行符，将 \n 替换为 <br/>
        formattedMsg.replace('\n', "<br/>");
        // 用户消息始终创建新块
        ui->AITextBrowser->append(QString("<div class='%1-msg'>%2</div>")
                                .arg(isUser ? "user" : "ai")
                                .arg(formattedMsg));
    } else {
        bool isFirstChunk = currentResponse.isEmpty();
        QTextCursor cursor(ui->AITextBrowser->document());
        cursor.movePosition(QTextCursor::End);

        // 检查上一个消息块的角色
        bool prevIsUser = false;
        if (cursor.block().previous().isValid()) {
            QString prevBlockText = cursor.block().previous().text();
            prevIsUser = prevBlockText.contains("user-msg");
        }

        // 当角色切换时，强制创建新的消息块并添加换行
        if (isUser != prevIsUser) {
            if(isFirstChunk) {
                 cursor.insertHtml(QString("<div class='%1-msg'>%2<br/></div>")
                            .arg(isUser ? "user" : "ai")
                            .arg(formattedMsg));
            }
            else{
            // 新消息块时保留换行符
            cursor.insertHtml(QString("<div class='%1-msg'><br/>%2<br/></div>")
                            .arg(isUser ? "user" : "ai")
                            .arg(formattedMsg));
            }
        } else {
            // 精确移动到最后一个同角色消息块的 HTML 内容末尾
            cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, 1);
            cursor.movePosition(QTextCursor::EndOfBlock);
            
            // 处理换行逻辑
            QString processedMsg = formattedMsg.replace('\n', "<br/>");
            cursor.insertHtml(processedMsg);
        }
    }
    
    // 保持滚动条在底部
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
        // 深色模式样式,有需要再开
        QString styleSheet = 
            //"div.user-msg {"
            //"   background: linear-gradient(135deg, #1565C0 0%, #0D47A1 100%);"
            //"   color: rgba(255,255,255,0.95);"
            //"   border-radius: 18px 18px 0 18px;"
            //"   padding: 14px 18px;"
            //"   margin: 8px 0 8px 25%;"
            //"   box-shadow: 0 4px 12px rgba(0,0,0,0.2);"
            //"   transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);"
            //"}"
            //"div.ai-msg {"
            //"   background: linear-gradient(135deg, #2c3e50 0%, #1a252f 100%);"
            //"   color: rgba(236,240,241,0.95);"
           // "   border-radius: 18px 18px 18px 0;"
            //"   padding: 14px 18px;"
            //"   margin: 8px 25% 8px 0;"
            //"   box-shadow: 0 4px 12px rgba(0,0,0,0.15);"
            //"   transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);"
            //"}"
            //"div.ai-msg::before, div.user-msg::before {"
            //"   content: '';"
            //"   position: absolute;"
            //"   width: 0;"
            //"   height: 0;"
            //"   border: 8px solid transparent;"
            //"   margin-top: 10px;"
           // "}"
           // "div.ai-msg::before {"
           // "   border-right-color: #1a252f;"
           // "   left: -16px;"
           // "}"
           // "div.user-msg::before {"
           // "   border-left-color: #0D47A1;"
           // "   right: -16px;"
           // "}"
           // "div.ai-msg, div.user-msg {"
           // "   max-width: 75%;"
           // "   font-size: 16px;"
           // "   line-height: 1.7;"
           // "   font-family: 'Segoe UI', 'PingFang SC', 'Microsoft YaHei', sans-serif;"
           // "   position: relative;"
           // "   word-wrap: break-word;
            //"}";
            "";
        ui->AITextBrowser->document()->setDefaultStyleSheet(styleSheet);
        ui->AITextBrowser->setStyleSheet(""); // 强制刷新样式
        
    } else {
        QMessageBox::information(this, "提示", "浅色模式已启用");
        // 浅色模式样式,有需要再开
        QString styleSheet =
           // "div.user-msg {"
            //"   background: linear-gradient(135deg, #3498db 0%, #2980b9 100%);"
            //"   color: rgba(0, 0, 0, 0.95);"  // 保持用户消息文字为黑色
            //"   border-radius: 18px 18px 0 18px;"
            //"   padding: 14px 18px;"
            //"   margin: 8px 0 8px 25%;"
            //"   box-shadow: 0 4px 12px rgba(0,0,0,0.1);"
            //"   transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);"
            //"}"
            //"div.ai-msg {"
            //"   background: linear-gradient(135deg, #ecf0f1 0%, #dfe6e9 100%);"
            //"   color: rgba(27, 38, 49, 0.95);"  // AI消息保持深色文字
            //"   border-radius: 18px 18px 18px 0;"
            //"   padding: 14px 18px;"
            //"   margin: 8px 25% 8px 0;"
            //"   box-shadow: 0 4px 12px rgba(0,0,0,0.05);"
            //"   transition: all 0.3s cubic-bezier(0.25, 0.8, 0.25, 1);"
            //"}"
            //"div.ai-msg::before, div.user-msg::before {"
            //"   content: '';"
            //"   position: absolute;"
            //"   width: 0;"
            //"   height: 0;"
            //"   border: 8px solid transparent;"
            //"   margin-top: 10px;"
            //"}"
           // "div.ai-msg::before {"
           // "   border-right-color: #85888aff;"
            //"   left: -16px;"
            //"}"
            //"div.user-msg::before {"
           // "   border-left-color: #1e5d87ff;"
           // "   right: -16px;"
           // "}"
           // "div.ai-msg, div.user-msg {"
           // "   max-width: 75%;"
           // "   font-size: 16px;"
           // "   line-height: 1.7;"
           // "   font-family: 'Segoe UI', 'PingFang SC', 'Microsoft YaHei', sans-serif;"
           // "   position: relative;"
          //  "   word-wrap: break-word;"
          //  "}";
          "";
        ui->AITextBrowser->document()->setDefaultStyleSheet(styleSheet);
        ui->AITextBrowser->setStyleSheet(""); // 强制刷新样式
    }
    
    // 强制重新渲染所有内容
    ui->AITextBrowser->document()->setPlainText(ui->AITextBrowser->toPlainText());
}

// 添加设置系统提示词的方法
void AIAgent::setSystemPrompt(const QString &prompt)
{
    systemPrompt = prompt;
}