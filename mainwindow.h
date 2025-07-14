#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "weathermod.h"
#include "daymod.h"
#include "memmod.h"
#include "settingmod.h"
#include "temperaturechart.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QDialog>
#include <QMessageBox>
#include <QbuttonGroup>

#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QDebug>
#include <QPixmap>
#include <QThread>
#include <QTimer>

#include <QTableView>
#include <QSqlTableModel>
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateWeatherInfo(const QString &temp,
                                const QString &feelsLike,
                                const QString &weathercode,
                                const QString &weathertext,
                                const QString &windDir,
                                const QString &windScale,
                                const QString &humidity,
                                const QString &updateTime);
    void updateWeather2Info(const QString &updateTime2,
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
    void on_updateWeatherButton_clicked();
    
private:
    Ui::MainWindow *ui;
    QButtonGroup *buttonGroup;
    WeatherMod *weatherMod;
    DayMod *dayMod;
    MemMod *memMod;
    SettingMod *settingMod;
};

#endif // MAINWINDOW_H
