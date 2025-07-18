#ifndef WEATHERMOD_H
#define WEATHERMOD_H

#include <QWidget>

//
#include <QNetworkAccessManager>
#include <QNetworkReply>

//
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>

//
#include <QString>
//
#include <QFont>
#include <QPalette>
#include <QBrush>
#include <QColor>

//
#include <QDateTime>

//
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

//
#include <QDebug>

//
#include <QUrl>
#include <QUrlQuery>

//图像
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>

#include <zlib.h>

class WeatherMod : public QWidget
{
    Q_OBJECT

public:
    explicit WeatherMod(QWidget *parent = nullptr);
    bool unzipData(QByteArray &compressedData, QByteArray &uncompressedData);
    void getWeatherData1();
    void parseWeatherData1(const QByteArray &data);
    void getWeatherData2();
    void parseWeatherData2(const QByteArray &data);
    void getlocationData();
    bool parselocationData(const QByteArray &data);
    bool isNetworkAvailable();
    QString location;//城市名字
    ~WeatherMod();
// 在signals部分添加
signals:
    void weather1DataUpdated(const QString &temp,
                            const QString &feelsLike,
                            const QString &weathercode,
                            const QString &weathertext,
                            const QString &windDir,
                            const QString &windScale,
                            const QString &humidity,
                            const QString &updateTime);

    void weather2DataUpdated(const QString &updateTime2,
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
                            const QList<QString> &humidity2);
    void locationDataUpdated(const QString &location);
private:
    QString apiKey;
    QString locationcode;//城市代码
    QString temp;//温度
    QString feelsLike;//体感温度
    QString weathercode;//天气代码
    QString weathertext;//天气文字描述
    QString windDir;//风向
    QString windScale;//风力
    QString humidity;//湿度
    QString updateTime;//更新时间

    QString updateTime2;//更新时间2
    QList<QString> fxDate;//预报日期
    QList<QString> sunrise;//日出时间
    QList<QString> sunset;//日落时间
    QList<QString> moonrise;//月升时间
    QList<QString> moonset;//月落时间
    QList<QString> moonPhase;//月相
    QList<QString> moonCode;//月相图标码
    QList<QString> tempMax;//最高温度
    QList<QString> tempMin;//最低温度
    QList<QString> iconDay;//白天图标
    QList<QString> textDay;//白天文字描述
    QList<QString> windDir2;//风向
    QList<QString> windScale2;//风力
    QList<QString> humidity2;//湿度
    
    //变量

    QNetworkAccessManager *manager;
    QUrlQuery query;
    bool networkAvailable;
    
    //组件
private slots:
    void handleReply(QNetworkReply *reply);
};
#endif // WEATHERMOD_H
