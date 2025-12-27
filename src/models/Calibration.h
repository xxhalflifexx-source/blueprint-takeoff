#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QPointF>
#include <QJsonObject>

/**
 * @brief Stores calibration data for converting pixels to real-world units.
 * 
 * Includes the two calibration points and the computed pixels-per-inch scale.
 */
class Calibration
{
public:
    Calibration();

    /**
     * @brief Set calibration from two points and a known distance.
     * @param p1 First calibration point (in pixels)
     * @param p2 Second calibration point (in pixels)
     * @param realDistanceInches Known distance between points in inches
     */
    void calibrate(const QPointF& p1, const QPointF& p2, double realDistanceInches);

    /**
     * @brief Get the pixels per inch scale factor.
     * @return Pixels per inch
     */
    double pixelsPerInch() const;

    /**
     * @brief Check if calibration has been performed.
     * @return true if calibrated
     */
    bool isCalibrated() const;

    /**
     * @brief Get the first calibration point.
     * @return Point 1 in image coordinates
     */
    QPointF point1() const;

    /**
     * @brief Get the second calibration point.
     * @return Point 2 in image coordinates
     */
    QPointF point2() const;

    /**
     * @brief Get the real distance used for calibration.
     * @return Distance in inches
     */
    double realDistanceInches() const;

    /**
     * @brief Reset calibration to default state.
     */
    void reset();

    /**
     * @brief Serialize calibration to JSON.
     * @return JSON object representation
     */
    QJsonObject toJson() const;

    /**
     * @brief Deserialize calibration from JSON.
     * @param json JSON object to read from
     * @return true if successful
     */
    bool fromJson(const QJsonObject& json);

private:
    double m_pixelsPerInch;
    bool m_calibrated;
    QPointF m_point1;
    QPointF m_point2;
    double m_realDistanceInches;
};

#endif // CALIBRATION_H
