#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <vector>
#include "Location.h"

class MapWidget : public QWidget {
    Q_OBJECT
public:
    explicit MapWidget(QWidget* parent = nullptr);

    void setMap(GameMap* m);
    void setCurrentLocation(int idx);
    std::vector<int> getReachable() const;

signals:
    void locationClicked(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    GameMap* map;
    int currentIdx;

    QPointF gridToPixel(int gx, int gy) const;
    int pixelToLocation(const QPointF& pt) const;

    static const int GRID_SIZE = 55;
    static const int MARGIN = 40;
};

#endif
