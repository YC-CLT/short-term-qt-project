#include "settingmod.h"

SettingMod::SettingMod(QWidget *parent, Ui::MainWindow* ui): QWidget(parent), ui(ui)
{
    connect(ui->checkDayLogButton, &QPushButton::clicked, this, &SettingMod::showDayLog);
    connect(ui->checkMemLogButton, &QPushButton::clicked, this, &SettingMod::showMemLog);
    
    // 连接背景图选择框
    connect(ui->picComboBox, QOverload<int>::of(&QComboBox::activated),this, &SettingMod::setBackground);
   connect(ui->clearPicCacheButton, &QPushButton::clicked, this, &SettingMod::clearPicCache);
}
// 单独提取的设置背景方法
void SettingMod::setBackground(int index)
{
    QStringList extensions = {".jpg", ".png", ".jpeg", ".gif", ".bmp", ".webp"};
    QString bgPath;
    QPalette palette;

    switch(index) {
        case 0:{
            // 恢复样式表定义的渐变背景
            parentWidget()->setStyleSheet(""); // 先清除动态设置的样式
            parentWidget()->setPalette(QPalette()); // 重置调色板
            break;
        }
        case 1:{
            int randomNum = QRandomGenerator::global()->bounded(2); // 生成0或1
            bgPath = QString(":/resources/background/wallpaper5%1.png").arg(randomNum + 1);
            if (!bgPath.isEmpty() && QFile::exists(bgPath))
                applyBackground(bgPath);
            break;
        }
        case 2:{
            // 尝试加载不同格式的图片
            int randomNum = QRandomGenerator::global()->bounded(2); // 生成0或1
            for (const auto& ext : extensions) {
                bgPath = QString(":/resources/background/wallpaper%1%2").arg(randomNum + 49).arg(ext);
                if (QFile::exists(bgPath)) {
                    applyBackground(bgPath);
                    return; // 找到图片后立即返回
                }
            }
            break;
        }
        case 3:{
            int randomNum = QRandomGenerator::global()->bounded(19);
            for (const auto& ext : extensions) {
                bgPath = QString(":/resources/background/wallpaper%1%2").arg(randomNum + 4).arg(ext);
                if (QFile::exists(bgPath)) {
                    applyBackground(bgPath);
                    return;
                }
            }
            break;
        }
        case 4:{
            int randomNum = QRandomGenerator::global()->bounded(26);
            for (const auto& ext : extensions) {
                bgPath = QString(":/resources/background/wallpaper%1%2").arg(randomNum + 23).arg(ext);
                if (QFile::exists(bgPath)) {
                    applyBackground(bgPath);
                    return;
                }
            }
            break;
        }
        case 5:{
            int randomNum = QRandomGenerator::global()->bounded(3);
            for (const auto& ext : extensions) {
                bgPath = QString(":/resources/background/wallpaper%1%2").arg(randomNum + 1).arg(ext);
                if (QFile::exists(bgPath)) {
                    applyBackground(bgPath);
                    return;
                }
            }
            break;
        }
        case 6:{
            int randomNum = QRandomGenerator::global()->bounded(53);
            for (const auto& ext : extensions) {
                bgPath = QString(":/resources/background/wallpaper%1%2").arg(randomNum + 1).arg(ext);
                if (QFile::exists(bgPath)) {
                    applyBackground(bgPath);
                    return;
                }
            }
            break;
        }
        case 7:
            bgPath = QFileDialog::getOpenFileName(this, 
            tr("选择背景图片"), 
            QDir::homePath(), 
            tr("图片文件 (*.jpg *.png *.jpeg *.gif *.bmp *.webp)"));
                if(!bgPath.isEmpty()) 
                applyBackground(bgPath);
            break;
        case 8:{
            // 网络API获取图片
            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            
            // 创建超时定时器
            QTimer *timeoutTimer = new QTimer(this);
            timeoutTimer->setSingleShot(true);
            timeoutTimer->start(30000); // 30秒超时
            
            connect(timeoutTimer, &QTimer::timeout, [this, manager]() {
                QMessageBox::warning(this, "错误", "网络请求超时");
                manager->deleteLater();
            });
            
            connect(manager, &QNetworkAccessManager::finished, this, [this, manager, timeoutTimer](QNetworkReply *reply) {
                timeoutTimer->stop(); // 请求完成时停止计时器
                timeoutTimer->deleteLater();
                
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray imageData = reply->readAll();
                    
                    // 创建缓存目录
                    QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
                    if (!cacheDir.exists()) {
                        cacheDir.mkpath(".");
                    }
                    
                    // 生成唯一文件名
                    QString cachePath = cacheDir.filePath("network_bg_" + 
                        QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".jpg");
                    
                    // 保存到缓存
                    QFile file(cachePath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(imageData);
                        file.close();
                        
                        // 应用背景
                        applyBackground(cachePath);
                    }
                } else {
                    QMessageBox::warning(this, "错误", "无法获取网络图片");
                }
                
                reply->deleteLater();
                manager->deleteLater();
            });
            
            // 这里替换为你的图片API URL
            QUrl imageUrl("https://t.alcy.cc/ycy");
            manager->get(QNetworkRequest(imageUrl));
            break;
        }
        default: parentWidget()->setPalette(palette);
            break;
    }     
}

void SettingMod::applyBackground(const QString& bgPath)
{
    QPixmap bg(bgPath);
    if(!bg.isNull()) {
        // 使用样式表设置背景图片，确保覆盖全局样式
        parentWidget()->setStyleSheet(QString(
            "QMainWindow {"
            "border-image: url(%1) 0 0 0 0 stretch stretch;"
            "background-position: center;"
            "}").arg(bgPath));
        parentWidget()->setAutoFillBackground(true);
    }
}

void SettingMod::showDayLog()
{   
    dayLogDialog = new QDialog(this);
    dayLogDialog->setWindowTitle("重要日日志");
    dayLogDialog->resize(600, 400);
    
    // 创建布局和TableView
    QVBoxLayout *layout = new QVBoxLayout(dayLogDialog);
    QTableView* dayTableView = new QTableView();
    QSqlTableModel* dayModel = new QSqlTableModel(dayLogDialog, QSqlDatabase::database("daymod_connection"));
    dayModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    dayTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    dayModel->setTable("operation_logs");  
    dayModel->select();
    
    dayTableView->setModel(dayModel);
    // 替换原有的居中设置
    dayTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    dayTableView->setItemDelegate(new CenterAlignDelegate(this));
    dayTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    dayTableView->setColumnWidth(0, 20);
    dayTableView->setColumnWidth(1, 60);
    dayTableView->setColumnWidth(2, 80);
    dayTableView->setColumnWidth(3, 150);
    dayTableView->setColumnWidth(4, 300);

    // 添加清空按钮
    QPushButton* clearDayButton = new QPushButton("清空日志");
    connect(clearDayButton, &QPushButton::clicked, [dayModel]() {
        dayModel->removeRows(0, dayModel->rowCount());
        dayModel->submitAll();
    });

    layout->addWidget(dayTableView);
    layout->addWidget(clearDayButton);  // 添加按钮到布局
    dayLogDialog->setLayout(layout);
    
    dayLogDialog->show();
}

void SettingMod::showMemLog()
{
    memLogDialog = new QDialog(this);
    memLogDialog->setWindowTitle("备忘录日志");
    memLogDialog->resize(600, 400);
    
    // 创建布局和TableView
    QVBoxLayout *layout = new QVBoxLayout(memLogDialog);
    QTableView* memTableView = new QTableView();
    QSqlTableModel* memModel = new QSqlTableModel(memLogDialog, QSqlDatabase::database("memmod_connection"));
    
    memModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    memTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    memModel->setTable("mem_log");  
    memModel->select();
    
    memTableView->setModel(memModel);
    // 替换原有的居中设置
    memTableView->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    memTableView->setItemDelegate(new CenterAlignDelegate(this));
    memTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    memTableView->setColumnWidth(0, 20);
    memTableView->setColumnWidth(1, 50);
    memTableView->setColumnWidth(2, 300);
    memTableView->setColumnWidth(3, 200);

    // 添加清空按钮
    QPushButton* clearMemButton = new QPushButton("清空日志");
    connect(clearMemButton, &QPushButton::clicked, [memModel]() {
        memModel->removeRows(0, memModel->rowCount());
        memModel->submitAll();
    });

    layout->addWidget(memTableView);
    layout->addWidget(clearMemButton);  // 添加按钮到布局
    memLogDialog->setLayout(layout);
    
    memLogDialog->show();
}

void SettingMod::clearPicCache()
{
    // 清除缓存目录
    QDir cacheDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    if (cacheDir.exists()) {
        cacheDir.removeRecursively();
    }
    QMessageBox::information(this, "提示", "图片缓存已清除");
}

SettingMod::~SettingMod()
{
    delete ui;
}
