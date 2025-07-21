#include "temperaturechart.h"
#include <QPainter>
#include <algorithm>

TemperatureChart::TemperatureChart(QWidget *parent)
    : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumSize(200, 150);
}

void TemperatureChart::setData(const QList<float> &highData, const QList<float> &lowData)
{
    // 添加空数据保护
    if(highData.isEmpty() || lowData.isEmpty()) return;

    highTempData = highData;
    lowTempData = lowData;
    
    auto combined = highData + lowData;
    yMin = *std::min_element(combined.constBegin(), combined.constEnd());
    yMax = *std::max_element(combined.constBegin(), combined.constEnd());
    
    // 添加范围有效性检查
    if(yMin >= yMax) {
        yMin = 0;
        yMax = 40; // 设置默认范围
    } else {
        float rangeBuffer = qMax((yMax - yMin) * 0.1f, 2.0f);
        yMin = qFloor(yMin - rangeBuffer);
        yMax = qCeil(yMax + rangeBuffer);
    }
    update();
}

void TemperatureChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 定义绘图区域
    const int horizontalPadding = 32;
    const int verticalPadding = 20;
    QRect chartRect = rect().adjusted(horizontalPadding, verticalPadding, -10, -verticalPadding);

    // 绘制背景和坐标轴
    painter.fillRect(rect(), QColor(240, 240, 240));
    painter.setPen(QPen(Qt::darkGray, 2));
    painter.drawLine(chartRect.bottomLeft(), chartRect.bottomRight());

    // 绘制背景和坐标轴后添加Y轴刻度
    painter.setPen(QPen(Qt::darkGray, 1));
    const int yTickCount = 5;
    for (int i = 0; i <= yTickCount; ++i) {
        float value = yMin + (yMax - yMin) * i / yTickCount;
        float yPos = chartRect.bottom() - (chartRect.height() * i / yTickCount);
        
        // 绘制刻度线
        painter.drawLine(chartRect.left() + 700, yPos, chartRect.left(), yPos);
        // 绘制刻度值
        painter.drawText(QRectF(chartRect.left() - horizontalPadding , yPos - 10, 
                              horizontalPadding , 12), 
                       Qt::AlignRight | Qt::AlignVCenter, 
                       QString::number(value, 'f', 1));
    }

    // 数据有效性检查
    if (highTempData.size() < 2 || lowTempData.size() < 2) {
        painter.drawText(rect(), Qt::AlignCenter, tr("数据不足"));
        return;
    }

    // 绘制双折线
    drawTemperatureLine(painter, chartRect, highTempData, highColor);
    drawTemperatureLine(painter, chartRect, lowTempData, lowColor);

    // 绘制图例
    drawLegend(painter, chartRect);
}

void TemperatureChart::drawTemperatureLine(QPainter& painter, const QRect& chartRect, 
                                         const QList<float>& data, const QColor& color)
{
    if(data.size() < 2) return;

    QVector<QPointF> points;
    const float divisor = data.size() - 1;
    const float xStep = chartRect.width() / qMax(divisor, 1.0f);

    for (int i = 0; i < data.size(); ++i) {
        float x = chartRect.left() + (chartRect.width() * i) / divisor;
        float y = chartRect.bottom() - ((data[i] - yMin) / (yMax - yMin)) * chartRect.height();
        points << QPointF(x, y);
    }

    // 绘制折线和数据点
    QPen linePen(color, 2);
    painter.setPen(linePen);
    painter.drawPolyline(points.constData(), points.size());

    painter.setBrush(Qt::white);
    for (const auto& point : points) {
        painter.drawEllipse(point, 4, 4);
    }
}

void TemperatureChart::drawLegend(QPainter& painter, const QRect& chartRect)
{
    const int legendPadding = 2;  // 缩小内边距
    const int legendHeight = 25;  // 缩小高度
    const int legendWidth = 80;  // 缩小宽度
    
    QRect legendRect(
        chartRect.right() - legendWidth,  // 更靠近右侧边缘
        chartRect.top() - 20,                  // 更靠近顶部
        legendWidth,
        legendHeight
    );

    // 绘制图例背景
    painter.fillRect(legendRect, QColor(255, 255, 255, 200));
    
    // 绘制图例项
    int yPos = legendRect.top() + legendPadding;
    painter.fillRect(QRect(legendRect.left() + legendPadding, yPos, 16, 16), highColor);
    painter.drawText(QRect(legendRect.left() + legendPadding + 20, yPos, 70, 16),
                   Qt::AlignLeft | Qt::AlignVCenter, "最高温度");

    yPos += 20;  // 减小行间距
    painter.fillRect(QRect(legendRect.left() + legendPadding, yPos, 16, 16), lowColor);
    painter.drawText(QRect(legendRect.left() + legendPadding + 20, yPos, 70, 16),
                   Qt::AlignLeft | Qt::AlignVCenter, "最低温度");
}
