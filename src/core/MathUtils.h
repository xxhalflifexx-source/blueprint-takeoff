#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <QPointF>
#include <QVector>

/**
 * @brief Utility class for mathematical operations on points.
 */
class MathUtils
{
public:
    /**
     * @brief Calculate the Euclidean distance between two points.
     * @param p1 First point
     * @param p2 Second point
     * @return Distance in pixels (or whatever unit the points are in)
     */
    static double distance(const QPointF& p1, const QPointF& p2);

    /**
     * @brief Calculate the total length of a polyline defined by multiple points.
     * @param points Vector of points forming the polyline
     * @return Total length of all segments
     */
    static double polylineLength(const QVector<QPointF>& points);
};

#endif // MATHUTILS_H

