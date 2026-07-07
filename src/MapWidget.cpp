#include "MapWidget.h"
#include <QFont>
#include <cmath>

MapWidget::MapWidget(QWidget* parent)
    : QWidget(parent), map(nullptr), currentIdx(0) {
    setMinimumSize(600, 520);
    setMouseTracking(true);
}

void MapWidget::setMap(GameMap* m) { map = m; update(); }
void MapWidget::setCurrentLocation(int idx) { currentIdx = idx; update(); }
std::vector<int> MapWidget::getReachable() const {
    if (!map) return {};
    return map->getReachableLocations();
}

QPointF MapWidget::gridToPixel(int gx, int gy) const {
    return QPointF(MARGIN + gx * GRID_SIZE + GRID_SIZE/2,
                   height() - MARGIN - (10 - gy) * GRID_SIZE - GRID_SIZE/2);
}

int MapWidget::pixelToLocation(const QPointF& pt) const {
    if (!map) return -1;
    for (size_t i = 0; i < map->locations.size(); ++i) {
        QPointF center = gridToPixel(map->locations[i].x, map->locations[i].y);
        double dx = pt.x() - center.x();
        double dy = pt.y() - center.y();
        if (sqrt(dx*dx + dy*dy) < GRID_SIZE/2) {
            return (int)i;
        }
    }
    return -1;
}

void MapWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), QColor("#F5F0E8"));

    if (!map) return;

    // Draw connection lines
    QPen linePen(QColor("#8D6E63"), 2.5);
    linePen.setCapStyle(Qt::RoundCap);
    for (size_t i = 0; i < map->locations.size(); ++i) {
        QPointF p1 = gridToPixel(map->locations[i].x, map->locations[i].y);
        for (int c : map->locations[i].connections) {
            if (c > (int)i) {  // Draw each line only once
                QPointF p2 = gridToPixel(map->locations[c].x, map->locations[c].y);
                // Draw dashed for inactive, solid for path from current
                bool active = (i == currentIdx || c == currentIdx);
                QPen lp = linePen;
                if (!active) {
                    lp.setStyle(Qt::DashLine);
                    lp.setColor(QColor("#BCAAA4"));
                } else {
                    lp.setColor(QColor("#FF6F00"));
                    lp.setWidth(3);
                }
                p.setPen(lp);
                p.drawLine(p1, p2);
            }
        }
    }

    // Draw locations
    for (size_t i = 0; i < map->locations.size(); ++i) {
        QPointF center = gridToPixel(map->locations[i].x, map->locations[i].y);
        bool isCurrent = (i == (size_t)currentIdx);
        bool isReachable = false;
        for (int c : map->locations[currentIdx].connections) {
            if (c == (int)i) isReachable = true;
        }

        // Circle
        int r = isCurrent ? 26 : 22;
        if (isCurrent) {
            // Glow effect
            p.setBrush(QColor(255, 183, 77, 60));
            p.setPen(Qt::NoPen);
            p.drawEllipse(center, r+6, r+6);
        }

        p.setPen(QPen(isCurrent ? QColor("#E65100") :
                      (isReachable ? QColor("#FF9800") : QColor("#78909C")), 3));
        // Location color
        if (map->locations[i].hasRest)
            p.setBrush(isCurrent ? QColor("#A5D6A7") : QColor("#C8E6C9"));
        else if (map->locations[i].hasShop)
            p.setBrush(isCurrent ? QColor("#FFCC80") : QColor("#FFE0B2"));
        else
            p.setBrush(isCurrent ? QColor("#90CAF9") : QColor("#BBDEFB"));

        p.drawEllipse(center, r, r);

        // Icon
        QFont iconFont("Segoe UI Emoji", 16);
        p.setFont(iconFont);
        p.drawText(QRectF(center.x()-r, center.y()-r, r*2, r*2),
                   Qt::AlignCenter, QString::fromStdString(map->locations[i].icon));

        // Name
        QFont nameFont("Microsoft YaHei", 9);
        p.setFont(nameFont);
        p.setPen(isCurrent ? QColor("#1A237E") : QColor("#37474F"));
        p.drawText(QRectF(center.x()-40, center.y()+r+2, 80, 18),
                   Qt::AlignCenter | Qt::AlignTop, QString::fromStdString(map->locations[i].name));
    }

    // Title
    QFont titleFont("Microsoft YaHei", 14, QFont::Bold);
    p.setFont(titleFont);
    p.setPen(QColor("#3E2723"));
    p.drawText(QRectF(0, 5, width(), 30), Qt::AlignCenter,
               "🏫 校园地图 — 点击可到达的位置移动");
}

void MapWidget::mousePressEvent(QMouseEvent* event) {
    int idx = pixelToLocation(event->pos());
    if (idx < 0 || idx == currentIdx) return;

    // Check if reachable
    bool reachable = false;
    for (int c : map->locations[currentIdx].connections) {
        if (c == idx) { reachable = true; break; }
    }
    if (reachable) {
        emit locationClicked(idx);
    }
}
