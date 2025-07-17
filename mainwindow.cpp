#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    static bool isDarkMode = true;
    // 添加灯泡按钮连接
    connect(ui->bulbButton, &QPushButton::clicked, this, [this]() {
        isDarkMode = !isDarkMode;
        loadStyleSheet(isDarkMode ? ":/style/styles.qss" : ":/style/light_styles.qss");
        emit themeChanged(isDarkMode);
    });
    this->setWindowTitle("未渊玄枢");
    this->setWindowIcon(QIcon(":/resources/icons/main.ico"));
    this->setMinimumSize(1280,720);
    this->resize(1280,720);

    // 加载QSS样式表
    loadStyleSheet(":/style/styles.qss");

    // 初始化按钮组
    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);  // 确保单选模式
    buttonGroup->addButton(ui->homePageButton, 0);
    buttonGroup->addButton(ui->weatherButton, 1);
    buttonGroup->addButton(ui->dayButton, 2);
    buttonGroup->addButton(ui->memButton, 3);
    buttonGroup->addButton(ui->settingButton, 4);

    // 切页面
    connect(buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
        [this](QAbstractButton* button) {
            ui->stackedWidget->setCurrentIndex(buttonGroup->id(button));
        });
    connect(ui->blankButton, &QPushButton::clicked, this, [this]() {
        bool isVisible = ui->stackedWidget->isVisible();
        ui->stackedWidget->setVisible(!isVisible);
        ui->AIWidget->setVisible(!isVisible);
    });
    //加载诗词
    sayingMod = new Saying(this, ui);
    //加载设置
    settingMod = new SettingMod(this, ui);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000); // 每秒更新

    // 添加默认选中首页
    buttonGroup->button(0)->setChecked(true);  // 设置首页按钮为选中状态
    ui->stackedWidget->setCurrentIndex(0);      // 显示第一个页面（首页）

    // 初始化天气模块
    weatherMod = new WeatherMod(this);
    connect(weatherMod, &WeatherMod::weather1DataUpdated, this, &MainWindow::updateWeatherInfo);
    connect(weatherMod, &WeatherMod::weather2DataUpdated, this, &MainWindow::updateWeather2Info);
    connect(weatherMod, &WeatherMod::locationDataUpdated, this, [this](const QString &location) {
        ui->todayWeatherLabel->setText("今日"+location+"天气：");
        ui->locationLabel->setText(location);
    });
    
    // 初始化重要日模块
    dayMod = new DayMod(this, ui);  

    connect(ui->dayRefreshButton, &QPushButton::clicked, this, [this]() {
        dayMod->refreshDayTable();
    });
    connect(ui->dayCommitButton, &QPushButton::clicked, this, [this]() {
        dayMod->commitDayTable();
    });
    connect(ui->dayDeleteButton, &QPushButton::clicked, this, [this]() {
        dayMod->deleteDayTable();
    });
    connect(ui->dayCancelButton, &QPushButton::clicked, this, [this]() {
        dayMod->cancelDayTable();
    });
    connect(ui->dayInsertButton, &QPushButton::clicked, this, [this]() {
        dayMod->insertDayTable();
    });

    memMod = new MemMod(this, ui);
    
    // 连接按钮信号
    connect(ui->memRefreshButton, &QPushButton::clicked, this, [this]() {
        memMod->refreshMemTable();
    });
    connect(ui->memCommitButton, &QPushButton::clicked, this, [this]() {
        memMod->commitMemTable();
    });
    connect(ui->memDeleteButton, &QPushButton::clicked, this, [this]() {
        memMod->deleteMemTable();
    });
    connect(ui->memCancelButton, &QPushButton::clicked, this, [this]() {
        memMod->cancelMemTable();
    });
    connect(ui->memInsertButton, &QPushButton::clicked, this, [this]() {
        memMod->insertMemTable();
    });

    // 初始化AI智能体
    aiAgent = new AIAgent(this, ui);
    
}

void MainWindow::updateWeatherInfo(const QString &temp,
                                const QString &feelsLike,
                                const QString &weathercode,
                                const QString &weathertext,
                                const QString &windDir,
                                const QString &windScale,
                                const QString &humidity,
                                const QString &updateTime)
{
    // 更新UI显示
    ui->currentTempLabel->setText(temp+"℃");
    ui->feelsLikeLabel->setText("体感温度："+feelsLike+"℃");
    ui->weatherTextLabel->setText(weathertext);
    ui->windLabel->setText(windDir + windScale+"级");
    // 优化后的天气提示逻辑
    int code = weathercode.toInt();
    int temperature = temp.toInt();
    int windLevel = windScale.toInt();
    QString tip;
    
    // 根据天气类型设置基础提示
    if(code >= 20 && code <= 22) { // 大风天气
        tip = QString("大风天气🌪️\n%1")
              .arg(windLevel >= 6 ? "避免高空作业" : 
                  (windLevel >= 4 ? "注意固定户外物品" : ""));
    } 
    else {
        switch(code) {
            case 100 ... 104: // 晴天
            case 150 ... 153:
                tip = "晴空万里☀️";
                break;
                
            case 300 ... 304: // 雷雨
                tip = "可能有雷电，注意安全⚡";
                break;
                
            case 305 ... 399: // 降雨
                tip = "记得携带雨伞☔";
                break;
                
            case 400 ... 499: // 降雪
                tip = (code == 404 || code == 405) ? 
                      "小心道路湿滑🌧️" : "注意防寒保暖🧣";
                break;
                
            case 500 ... 515: // 雾霾沙尘
                tip = (code >= 507 && code <= 508) ? 
                      "沙尘天气请防护🌪️" : "建议佩戴口罩😷";
                break;
                
            case 900: tip = "高温注意防暑🔥"; break;
            case 901: tip = "寒冷注意保暖❄️"; break;
            default:  tip = "今日天气适宜";
        }
    }
    
    // 温度附加提示（与天气类型提示互补）
    if(temperature >= 30 && !tip.contains("高温")) {
        tip += " 高温注意防晒";
    } 
    else if(temperature <= 5 && !tip.contains("寒冷")) {
        tip += " 低温注意保暖";
    }
    
    ui->tipLabel->setText(tip.trimmed());
    ui->humidityLabel->setText("湿度"+humidity+"%");
    ui->updateTimeLabel->setText("更新时间："+QDateTime::fromString(updateTime, Qt::ISODate).toString("yyyy-MM-dd HH:mm"));
    
    // 加载SVG图片并适应标签大小
    // 在updateWeatherInfo函数中修改：
    QSvgRenderer renderer(QString(":/resources/icons/%1.svg").arg(weathercode));
    if(renderer.isValid()) {
        QPixmap pixmap(ui->weatherIconLabel->size() * devicePixelRatio());
        pixmap.setDevicePixelRatio(devicePixelRatio());
        pixmap.fill(Qt::transparent);
        
        QPainter painter(&pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        
        // 先绘制白色背景
        painter.drawRect(pixmap.rect());
        // 再渲染SVG图标
        renderer.render(&painter);
        
        ui->weatherIconLabel->setPixmap(pixmap);
    }
    else {
        qDebug() << "SVG load failed:" << QString(":/resources/icons/%1.svg").arg(weathercode);
    }
}
// 24小时预报
void MainWindow::updateWeather2Info(const QString &updateTime2,
                                const QList<QString> &fxDate,
                                const QList<QString> &sunrise,
                                const QList<QString> &sunset,
                                const QList<QString> &moonrise,
                                const QList<QString> &moonset,
                                const QList<QString> &moonPhase,
                                const QList<QString> &moonCode,
                                const QList<QString> &tempMax,
                                const QList<QString> &tempMin,
                                const QList<QString> &iconDay,
                                const QList<QString> &textDay,
                                const QList<QString> &windDir2,
                                const QList<QString> &windScale2,
                                const QList<QString> &humidity2)
{
    
    // 更新UI显示
    ui->updateTime2Label->setText("更新时间："+QDateTime::fromString(updateTime2, Qt::ISODate).toString("yyyy-MM-dd HH:mm"));
    for(int i = 0; i < 7; i++) {
            
        findChild<QLabel*>(QString("fxDateLabel_%1").arg(i+1))->setText(QDateTime::fromString(fxDate[i], Qt::ISODate).toString("MM-dd"));
        findChild<QLabel*>(QString("sunriseLabel_%1").arg(i+1))->setText("日出"+sunrise[i]);
        findChild<QLabel*>(QString("sunsetLabel_%1").arg(i+1))->setText("日落"+sunset[i]);
        findChild<QLabel*>(QString("moonriseLabel_%1").arg(i+1))->setText("月出"+moonrise[i]);
        findChild<QLabel*>(QString("moonsetLabel_%1").arg(i+1))->setText("月落"+moonset[i]);
        findChild<QLabel*>(QString("moonPhaseLabel_%1").arg(i+1))->setText(moonPhase[i]);
        findChild<QLabel*>(QString("tempMaxLabel_%1").arg(i+1))->setText("最高"+tempMax[i]+"℃");
        findChild<QLabel*>(QString("tempMinLabel_%1").arg(i+1))->setText("最低"+tempMin[i]+"℃");
        findChild<QLabel*>(QString("windDir2Label_%1").arg(i+1))->setText(windDir2[i]);
        findChild<QLabel*>(QString("textDayLabel_%1").arg(i+1))->setText(textDay[i]);
        findChild<QLabel*>(QString("windScale2Label_%1").arg(i+1))->setText(windScale2[i]+"级");
        findChild<QLabel*>(QString("humidity2Label_%1").arg(i+1))->setText("湿度"+humidity2[i]+"%");

    }


    // 加载SVG图片并适应标签大小

    for(int i = 0; i < 7; i++) {
        QLabel* moonLabel = findChild<QLabel*>(QString("moonCodeLabel_%1").arg(i+1));
        if(moonLabel) {
            QSvgRenderer renderer(QString(":/resources/icons/%1.svg").arg(moonCode[i]));
            if(renderer.isValid()) {
                QPixmap pixmap(moonLabel->size() * devicePixelRatio());
                pixmap.setDevicePixelRatio(devicePixelRatio());
                pixmap.fill(Qt::transparent);
                
                QPainter painter(&pixmap);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setBrush(Qt::white);
                painter.setPen(Qt::NoPen);
                
                // 先填充白色
                painter.drawRect(pixmap.rect());
                // 再绘制图标
                renderer.render(&painter);
                moonLabel->setPixmap(pixmap);
            }
            else {
                qDebug() << "SVG load failed:" << QString(":/resources/icons/%1.svg").arg(moonCode[i]);
            }
        }
    }

    // 加载SVG图片并适应标签大小
    for(int i = 0; i < 7; i++) {
        QLabel* iconLabel = findChild<QLabel*>(QString("iconDayLabel_%1").arg(i+1));
        if(iconLabel) {
            QSvgRenderer renderer(QString(":/resources/icons/%1.svg").arg(iconDay[i]));
            if(renderer.isValid()) {
                QPixmap pixmap(iconLabel->size() * devicePixelRatio());
                pixmap.setDevicePixelRatio(devicePixelRatio());
                pixmap.fill(Qt::transparent);
                
                QPainter painter(&pixmap);
                painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                painter.setBrush(Qt::white);
                painter.setPen(Qt::NoPen);
                
                // 先填充白色
                painter.drawRect(pixmap.rect());
                // 再绘制图标
                renderer.render(&painter);
                iconLabel->setPixmap(pixmap);
            }
            else {
                qDebug() << "SVG load failed:" << QString(":/resources/icons/%1.svg").arg(iconDay[i]);
            }
        }
    }

    // 修改后的折线图绘制部分
    QList<float> highData, lowData;
    bool dataValid = true;

    // 添加数据有效性检查
    for (int i = 0; i < 7; ++i) {
        bool ok1, ok2;
        float high = tempMax[i].toFloat(&ok1);
        float low = tempMin[i].toFloat(&ok2);
        
        if(!ok1 || !ok2 || i >= tempMax.size() || i >= tempMin.size()) {
            qDebug() << "无效的温度数据，索引:" << i;
            dataValid = false;
            break;
        }
        highData << high;
        lowData << low;
    }

    if(dataValid) {
        // 安全清理旧图表
        if(QLayout* layout = ui->tempFrame->layout()) {
            QLayoutItem* item;
            while ((item = layout->takeAt(0)) != nullptr) {
                if (QWidget* widget = item->widget()) {
                    widget->deleteLater();
                }
                delete item;
            }
        } else {
            ui->tempFrame->setLayout(new QVBoxLayout());
        }

        // 创建新图表
        TemperatureChart *chart = new TemperatureChart(ui->tempFrame);
        chart->setData(highData, lowData);
        ui->tempFrame->layout()->addWidget(chart);
    }
}

MainWindow::~MainWindow()
{ 
    delete ui;
}



void MainWindow::on_updateWeatherButton_clicked()
{
    QString searchLocation = ui->locationLineEdit->text();
    if(searchLocation.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入城市名称");
        return;
    }
    else {
        weatherMod->location = searchLocation;
        weatherMod->getlocationData();
    }
}

void MainWindow::updateTime()
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm:ss");
    ui->timeLabel->setText(currentTime);
}

void MainWindow::loadStyleSheet(const QString &path) 
{
    QFile file(path);
    if(file.open(QIODevice::ReadOnly)) {
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    } else {
        QMessageBox::warning(this, "Warning", "样式表加载失败" + file.errorString());
    }
}