#ifndef TEMPERATURECHART_H
#define TEMPERATURECHART_H

#include <QWidget>
#include <QColor>
#include <QRect>

class TemperatureChart : public QWidget
{
    Q_OBJECT
public:
    explicit TemperatureChart(QWidget *parent = nullptr);
    void setData(const QList<float> &highData, const QList<float> &lowData);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void drawTemperatureLine(QPainter& painter, const QRect& chartRect, 
                            const QList<float>& data, const QColor& color);
    void drawLegend(QPainter& painter, const QRect& chartRect);

    QList<float> highTempData;
    QList<float> lowTempData;
    float yMin = 0;
    float yMax = 40;
    const QColor highColor = QColor(255, 79, 47);
    const QColor lowColor = QColor(47, 79, 255);
};

#endif // TEMPERATURECHART_H
