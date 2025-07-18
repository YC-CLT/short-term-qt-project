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
    networkAvailable = isNetworkAvailable();
    if(!networkAvailable) 
        QMessageBox::warning(this, "网络错误", "请检查网络连接");
    else    
        getlocationData();
}

void WeatherMod::getlocationData()
{   
    QUrl url("https://mq2tunb84h.re.qweatherapi.com/geo/v2/city/lookup");
    query.clear();
    query.addQueryItem("location", QUrl::toPercentEncoding(location)); // 添加 URL 编码
    url.setQuery(query);

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
                emit locationDataUpdated(location);
                return true;
            }
            else {
                QMessageBox::warning(this,"警告","未找到该城市");
            }
        }
        else {
            QString error = obj["code"].toString();
            QMessageBox::warning(this,"警告","错误码："+error);
            return false;
        }
    }
    else {
        QMessageBox::warning(this,"警告","JSON数据无效");
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

            emit weather1DataUpdated(temp, feelsLike, weathercode, weathertext, windDir, windScale, humidity, updateTime);
        }
        else {
            QString error = obj["code"].toString();
            QMessageBox::warning(this,"警告","错误码："+error);
        }
    }
    else {
        QMessageBox::warning(this,"警告","JSON数据无效");
    }
}

void WeatherMod::getWeatherData2(){
    QUrl url("https://mq2tunb84h.re.qweatherapi.com/v7/weather/7d");
    query.clear();
    query.addQueryItem("location", locationcode);
    query.addQueryItem("lang", "zh"); // 中文
    query.addQueryItem("unit", "m"); // 公制单位
    
    url.setQuery(query);

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
            }
            
            // 发射信号时传递QList
            emit weather2DataUpdated(updateTime2, fxDate, 
                                    sunrise, sunset, moonrise, moonset,
                                    moonPhase, moonCode, tempMax, tempMin,
                                    iconDay, textDay, windDir2, windScale2, humidity2);
        } else {
            QString error = obj["code"].toString();
            QMessageBox::warning(this,"警告","错误码："+error);
        }
    } else {
        QMessageBox::warning(this,"警告","JSON数据无效");
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
                    QMessageBox::warning(this,"警告","位置数据不可用");

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
        QMessageBox::warning(this,"警告","网络错误："+reply->errorString());

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
        QMessageBox::warning(this,"警告","解压初始化错误");

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
            QMessageBox::warning(this,"警告","解压错误");
            return false;
        }

        uncompressedData.append(buffer, CHUNK_SIZE - stream.avail_out);
    } while (ret != Z_STREAM_END);

    inflateEnd(&stream);
    return true;
}

bool WeatherMod::isNetworkAvailable()
{
    QTcpSocket socket;
    socket.connectToHost("www.baidu.com", 80); // 使用网站测试
    if(socket.waitForConnected(3000)) { // 3秒超时
        socket.close();
        return true;
    }
    return false;
}

WeatherMod::~WeatherMod() {
    delete manager;
}
