#include "saying.h"

Saying::Saying(QWidget *parent,Ui::MainWindow *ui): QWidget{parent},ui{ui}
{   
    token="jV8Lg1yHk58DvYK9RxVOFlObkF2bCSC6";
    manager = new QNetworkAccessManager(this);
    QUrl url("https://v2.jinrishici.com/sentence");
    QNetworkRequest request(url);
    request.setRawHeader("X-User-Token", token.toUtf8());
    reply = manager->get(request);
    connect(reply,&QNetworkReply::finished,this,&Saying::replyFinished);
}

void Saying::replyFinished()
{
    // 解析JSON数据
    jsonDoc = QJsonDocument::fromJson(reply->readAll());
    jsonObj = jsonDoc.object();
    
    // 检查状态是否为success
    if(jsonObj["status"].toString() == "success") {
        QJsonObject dataObj = jsonObj["data"].toObject();
        QString content = dataObj["content"].toString();

        // 重置光标状态
        m_showCursor = true;
        m_charPos = 0;
        m_typingContent = content;
        
        QTimer::singleShot(0, this, &Saying::typeNextChar);
    } else {
        // 处理错误情况
        int errCode = jsonObj["errCode"].toInt();
        QString errMessage = jsonObj["errMessage"].toString();
        QMessageBox::warning(this, "API错误", QString("错误码: %1\n%2").arg(errCode).arg(errMessage));
    }
}

void Saying::typeNextChar()
{
    // 更新显示文本（带闪烁光标）
    QString displayText = m_typingContent.left(m_charPos);
    ui->sayingLabel->setText(displayText + (m_showCursor ? "█" : ""));

    if (m_charPos < m_typingContent.length()) {
        m_charPos++;
        int delay = 180 + (QRandomGenerator::global()->bounded(90)-40);
        QTimer::singleShot(delay, this, &Saying::typeNextChar);
    } else {
        ui->sayingLabel->setText(displayText);
        m_showCursor = false;
        // 3秒后开始回退
        QTimer::singleShot(3000, this, [this]() {
            m_isBackspacing = true;
        QTimer::singleShot(0, this, &Saying::backspaceText);
        });
    }
}

// 新增回退函数
void Saying::backspaceText()
{
    if (m_charPos > 0) {
        m_charPos--;
        ui->sayingLabel->setText(m_typingContent.left(m_charPos) + (""));
        QTimer::singleShot(50, this, &Saying::backspaceText); // 快速回退
    } else {
        m_isBackspacing = false;
        // 回退完成后重新开始打字
        QTimer::singleShot(1500, this, [this]() {
            m_charPos = 0;
            m_showCursor = true;
            QTimer::singleShot(0, this, &Saying::typeNextChar);
        });
    }
}