#include "weathermod.h"

WeatherMod::WeatherMod(QWidget *parent)
    : QWidget{parent}
{   
    location = "郑州";
    apiKey = "3f75d437a4a54de4a0b8a3ac6fcf5b75";
    locationcode = "101180101";
    query = QUrlQuery();
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &WeatherMod::handleReply);
    getlocationData();
}

void WeatherMod::getlocationData()
{
    QUrl url("https://mq2tunb84h.re.qweatherapi.com/geo/v2/city/lookup");
    query.clear();
    query.addQueryItem("location", QUrl::toPercentEncoding(location)); // 添加 URL 编码
    url.setQuery(query);
    //qDebug() << url.toString().toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("Accept-Encoding", "gzip");
    request.setRawHeader("X-QW-Api-Key", apiKey.toUtf8());  
    manager->get(request);
}

bool WeatherMod::parselocationData(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj["code"].toString() == "200") {
            QJsonArray locations = obj["location"].toArray();
            if (!locations.isEmpty()) {
                QJsonObject firstCity = locations[0].toObject();
                location = firstCity["name"].toString();
                locationcode = firstCity["id"].toString();
                //qDebug() << "Selected city:" << location << "ID:" << locationcode;
                emit locationDataUpdated(location);
                return true;
            }
            else {
                //qDebug() << "No location data available";
            }
        }
        else {
            //qDebug() << "Error: " << obj["code"].toString();
            return false;
        }
    }
    else {
        //qDebug() << "Error: Invalid JSON data";
    }
    return false;
}

void WeatherMod::getWeatherData1()
{
    QUrl url("https://mq2tunb84h.re.qweatherapi.com/v7/weather/now");
    query.clear();
    query.addQueryItem("location", locationcode);
    query.addQueryItem("lang", "zh"); // 中文
    query.addQueryItem("unit", "m"); // 公制单位
    
    url.setQuery(query);
    //qDebug() << url.toString().toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("Accept-Encoding", "gzip");
    request.setRawHeader("X-QW-Api-Key", apiKey.toUtf8()); 
    manager->get(request);
}

void WeatherMod::parseWeatherData1(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj["code"].toString() == "200") {
            QJsonObject now = obj["now"].toObject();
            
            temp = now["temp"].toString();          // 温度
            feelsLike = now["feelsLike"].toString(); // 体感温度
            weathercode = now["icon"].toString();    // 天气代码
            weathertext = now["text"].toString();    // 天气文字描述
            windDir = now["windDir"].toString();     // 风向
            windScale = now["windScale"].toString(); // 风力
            humidity = now["humidity"].toString();   // 湿度
            updateTime = now["obsTime"].toString();  // 更新时间
            
            //qDebug() << "Current weather:"
                     //<< "Temp:" << temp
                     //<< "FeelsLike:" << feelsLike
                     //<< "Weather:" << weathertext
                    // << "Wind:" << windDir << windScale
                    // << "Humidity:" << humidity
                    // << "UpdateTime:" << updateTime
                    // << "weathercode:" << weathercode;

            emit weather1DataUpdated(temp, feelsLike, weathercode, weathertext, windDir, windScale, humidity, updateTime);
        }
        else {
            //qDebug() << "Error: " << obj["code"].toString();
        }
    }
    else {
        //qDebug() << "Error: Invalid JSON data";
    }
}

void WeatherMod::getWeatherData2(){
    QUrl url("https://mq2tunb84h.re.qweatherapi.com/v7/weather/7d");
    query.clear();
    query.addQueryItem("location", locationcode);
    query.addQueryItem("lang", "zh"); // 中文
    query.addQueryItem("unit", "m"); // 公制单位
    
    url.setQuery(query);
    //qDebug() << url.toString().toUtf8();

    QNetworkRequest request(url);
    request.setRawHeader("Accept-Encoding", "gzip");
    request.setRawHeader("X-QW-Api-Key", apiKey.toUtf8()); 
    manager->get(request);
}
void WeatherMod::parseWeatherData2(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj["code"].toString() == "200") {
            updateTime2 = obj["updateTime"].toString();
            QJsonArray daily = obj["daily"].toArray();
            
            // 遍历7天预报数据（最多处理7个元素）
            // 初始化所有QList
            fxDate.clear();
            sunrise.clear();
            sunset.clear();
            moonrise.clear();
            moonset.clear();
            moonPhase.clear();
            moonCode.clear();
            tempMax.clear();
            tempMin.clear();
            iconDay.clear();
            textDay.clear();
            windDir2.clear();
            windScale2.clear();
            humidity2.clear();
            
            for(int i = 0; i < daily.size() && i < 7; i++) {
                QJsonObject day = daily[i].toObject();
                fxDate.append(day["fxDate"].toString());
                sunrise.append(day["sunrise"].toString());
                sunset.append(day["sunset"].toString());
                moonrise.append(day["moonrise"].toString());
                moonset.append(day["moonset"].toString());
                moonPhase.append(day["moonPhase"].toString());
                moonCode.append(day["moonPhaseIcon"].toString());
                tempMax.append(day["tempMax"].toString());
                tempMin.append(day["tempMin"].toString());
                iconDay.append(day["iconDay"].toString());
                textDay.append(day["textDay"].toString());
                windDir2.append(day["windDirDay"].toString());
                windScale2.append(day["windScaleDay"].toString());
                humidity2.append(day["humidity"].toString());
                
                //qDebug() << "Day" << i+1 << "Forecast:";
                //qDebug() << "Date:" << fxDate[i];
                //qDebug() << "Sunrise:" << sunrise[i];
                //qDebug() << "Sunset:" << sunset[i];
                //qDebug() << "Moonrise:" << moonrise[i];
                //qDebug() << "Moonset:" << moonset[i];
                //qDebug() << "MoonPhase:" << moonPhase[i];
                //qDebug() << "MoonCode:" << moonCode[i];
                //qDebug() << "TempMax:" << tempMax[i];
                //qDebug() << "TempMin:" << tempMin[i];
                //qDebug() << "IconDay:" << iconDay[i];
                //qDebug() << "TextDay:" << textDay[i];
                //qDebug() << "WindDir2:" << windDir2[i];
                //qDebug() << "WindScale2:" << windScale2[i];
                //qDebug() << "Humidity2:" << humidity2[i];
            }
            
            // 发射信号时传递QList
            emit weather2DataUpdated(updateTime2, fxDate, 
                                    sunrise, sunset, moonrise, moonset,
                                    moonPhase, moonCode, tempMax, tempMin,
                                    iconDay, textDay, windDir2, windScale2, humidity2);
        } else {
            //qDebug() << "Error: " << obj["code"].toString();
        }
    } else {
        //qDebug() << "Error: Invalid JSON data";
    }
}

void WeatherMod::handleReply(QNetworkReply *reply)
{
    QByteArray uncompressedData;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray compressedData = reply->readAll();        
        if(unzipData(compressedData, uncompressedData)) { // 添加解压结果检查
            QString url = reply->url().toString();
            if(url.contains("city/lookup")) {
                if(parselocationData(uncompressedData)) {
                    getWeatherData1(); // 位置查询成功后再获取天气
                    getWeatherData2(); // 位置查询成功后再获取7天预报
                }
                else {
                    //qDebug() << "Error: Location data not available";

                }
            } 
            else if(url.contains("weather/now")) {
                parseWeatherData1(uncompressedData); // 天气数据解析
            }
            // 添加以下分支处理7天预报
            else if(url.contains("weather/7d")) { 
                parseWeatherData2(uncompressedData);
            }
        }
    } else {
        //qDebug() << "Error: " << reply->errorString();

    }
    reply->deleteLater();
}

bool WeatherMod::unzipData(QByteArray &compressedData, QByteArray &uncompressedData)
{
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = static_cast<uInt>(compressedData.size());
    stream.next_in = reinterpret_cast<Bytef*>(compressedData.data());

    // 使用更大的缓冲区
    int ret = inflateInit2(&stream, 16 + MAX_WBITS);
    if (ret != Z_OK) {
        //qDebug() << "inflateInit error:" << ret;

        return false;
    }

    constexpr int CHUNK_SIZE = 32768;
    char buffer[CHUNK_SIZE];
    uncompressedData.clear();

    do {
        stream.avail_out = CHUNK_SIZE;
        stream.next_out = reinterpret_cast<Bytef*>(buffer);
        ret = inflate(&stream, Z_SYNC_FLUSH);
        
        if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR) {
            //qDebug() << "inflate error:" << ret;
            return false;
        }

        uncompressedData.append(buffer, CHUNK_SIZE - stream.avail_out);
    } while (ret != Z_STREAM_END);

    inflateEnd(&stream);
    return true;
}

WeatherMod::~WeatherMod() {
    delete manager;
}
