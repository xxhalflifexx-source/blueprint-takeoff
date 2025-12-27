#ifndef COORDINATETRANSFORM_H
#define COORDINATETRANSFORM_H

#include <QPointF>
#include <QVector>

/**
 * @brief Handles conversion between pixel coordinates and real-world units (inches).
 * 
 * Uses a calibration factor (pixels per inch) to convert measurements.
 */
class CoordinateTransform
{
public:
    CoordinateTransform();

    /**
     * @brief Set the calibration scale factor.
     * @param pixelsPerInch Number of pixels per inch
     */
    void setPixelsPerInch(double pixelsPerInch);

    /**
     * @brief Get the current calibration scale factor.
     * @return Pixels per inch
     */
    double getPixelsPerInch() const;

    /**
     * @brief Check if calibration has been set (not default).
     * @return true if calibrated, false otherwise
     */
    bool isCalibrated() const;

    /**
     * @brief Convert a pixel distance to inches.
     * @param pixelDistance Distance in pixels
     * @return Distance in inches
     */
    double pixelsToInches(double pixelDistance) const;

    /**
     * @brief Convert inches to pixels.
     * @param inches Distance in inches
     * @return Distance in pixels
     */
    double inchesToPixels(double inches) const;

private:
    double m_pixelsPerInch;
    bool m_calibrated;
};

#endif // COORDINATETRANSFORM_H

