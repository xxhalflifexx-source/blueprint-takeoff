#include "MathUtils.h"
#include <cmath>

double MathUtils::distance(const QPointF& p1, const QPointF& p2)
{
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    return std::sqrt(dx * dx + dy * dy);
}

double MathUtils::polylineLength(const QVector<QPointF>& points)
{
    if (points.size() < 2) {
        return 0.0;
    }

    double totalLength = 0.0;
    for (int i = 1; i < points.size(); ++i) {
        totalLength += distance(points[i - 1], points[i]);
    }
    return totalLength;
}

