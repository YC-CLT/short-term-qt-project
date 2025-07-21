#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "./ui_mainwindow.h"

#include "weathermod.h"
#include "daymod.h"
#include "memmod.h"
#include "temperaturechart.h"
#include "saying.h"
#include "settingmod.h"
#include "uisetting.h"
#include "aiagent.h"

#include <QMainWindow>
#include <QStackedWidget>
#include <QDialog>
#include <QMessageBox>
#include <QbuttonGroup>
#include <QPushButton>

#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QDebug>
#include <QPixmap>
#include <QThread>
#include <QTimer>
#include <QEvent>

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
    void loadStyleSheet(const QString &path);
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
    void updateWeatherInfo2(const QString &updateTime2,
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
    void updateTime();
    
signals:
    void themeChanged(bool theme);

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QButtonGroup *buttonGroup;
    WeatherMod *weatherMod;
    DayMod *dayMod;
    MemMod *memMod;
    Saying *sayingMod;
    SettingMod *settingMod;
    UiSetting *uiManager;
    AIAgent *aiAgent;
    friend class UiSetting;
};

#endif // MAINWINDOW_H
