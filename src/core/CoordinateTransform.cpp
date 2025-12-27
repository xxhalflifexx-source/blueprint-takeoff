#include "CoordinateTransform.h"

CoordinateTransform::CoordinateTransform()
    : m_pixelsPerInch(1.0)  // Default: 1 pixel = 1 inch (uncalibrated)
    , m_calibrated(false)
{
}

void CoordinateTransform::setPixelsPerInch(double pixelsPerInch)
{
    if (pixelsPerInch > 0.0) {
        m_pixelsPerInch = pixelsPerInch;
        m_calibrated = true;
    }
}

double CoordinateTransform::getPixelsPerInch() const
{
    return m_pixelsPerInch;
}

bool CoordinateTransform::isCalibrated() const
{
    return m_calibrated;
}

double CoordinateTransform::pixelsToInches(double pixelDistance) const
{
    return pixelDistance / m_pixelsPerInch;
}

double CoordinateTransform::inchesToPixels(double inches) const
{
    return inches * m_pixelsPerInch;
}

