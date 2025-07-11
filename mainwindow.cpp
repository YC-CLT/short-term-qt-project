#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化按钮组
    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);  // 确保单选模式
    buttonGroup->addButton(ui->homePageButton, 0);
    buttonGroup->addButton(ui->weatherButton, 1);
    buttonGroup->addButton(ui->dayButton, 2);
    buttonGroup->addButton(ui->memButton, 3);
    buttonGroup->addButton(ui->aboutButton, 4);

    // 切页面
    connect(buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
        [this](QAbstractButton* button) {
            ui->stackedWidget->setCurrentIndex(buttonGroup->id(button));
        });

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
    ui->feelsLikeLabel->setText(feelsLike+"℃");
    ui->weatherTextLabel->setText(weathertext);
    ui->windLabel->setText(windDir + windScale+"级");
    ui->humidityLabel->setText(humidity+"%");
    ui->updateTimeLabel->setText("更新时间："+QDateTime::fromString(updateTime, Qt::ISODate).toString("yyyy-MM-dd HH:mm"));
    
    // 加载SVG图片并适应标签大小
    QSvgRenderer renderer(QString(":/resources/icons/%1.svg").arg(weathercode));
    if(renderer.isValid()) {
        QPixmap pixmap(ui->weatherIconLabel->size() * devicePixelRatio());
        pixmap.setDevicePixelRatio(devicePixelRatio());
        pixmap.fill(Qt::transparent);
        
        QPainter painter(&pixmap);
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

