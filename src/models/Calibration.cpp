#include "Calibration.h"
#include "MathUtils.h"

Calibration::Calibration()
    : m_pixelsPerInch(1.0)
    , m_calibrated(false)
    , m_point1()
    , m_point2()
    , m_realDistanceInches(0.0)
{
}

void Calibration::calibrate(const QPointF& p1, const QPointF& p2, double realDistanceInches)
{
    if (realDistanceInches <= 0.0) {
        return;
    }

    double pixelDistance = MathUtils::distance(p1, p2);
    if (pixelDistance > 0.0) {
        m_point1 = p1;
        m_point2 = p2;
        m_realDistanceInches = realDistanceInches;
        m_pixelsPerInch = pixelDistance / realDistanceInches;
        m_calibrated = true;
    }
}

double Calibration::pixelsPerInch() const
{
    return m_pixelsPerInch;
}

bool Calibration::isCalibrated() const
{
    return m_calibrated;
}

QPointF Calibration::point1() const
{
    return m_point1;
}

QPointF Calibration::point2() const
{
    return m_point2;
}

double Calibration::realDistanceInches() const
{
    return m_realDistanceInches;
}

void Calibration::reset()
{
    m_pixelsPerInch = 1.0;
    m_calibrated = false;
    m_point1 = QPointF();
    m_point2 = QPointF();
    m_realDistanceInches = 0.0;
}

void Calibration::setPixelsPerInch(double ppi)
{
    m_pixelsPerInch = ppi;
    m_calibrated = (ppi > 0.0);
}

void Calibration::setCalibrationPoints(const QPointF& p1, const QPointF& p2)
{
    m_point1 = p1;
    m_point2 = p2;
}

QJsonObject Calibration::toJson() const
{
    QJsonObject json;
    json["calibrated"] = m_calibrated;
    json["pixelsPerInch"] = m_pixelsPerInch;
    json["point1_x"] = m_point1.x();
    json["point1_y"] = m_point1.y();
    json["point2_x"] = m_point2.x();
    json["point2_y"] = m_point2.y();
    json["realDistanceInches"] = m_realDistanceInches;
    return json;
}

bool Calibration::fromJson(const QJsonObject& json)
{
    if (!json.contains("calibrated")) {
        return false;
    }

    m_calibrated = json["calibrated"].toBool();
    m_pixelsPerInch = json["pixelsPerInch"].toDouble(1.0);
    m_point1 = QPointF(json["point1_x"].toDouble(), json["point1_y"].toDouble());
    m_point2 = QPointF(json["point2_x"].toDouble(), json["point2_y"].toDouble());
    m_realDistanceInches = json["realDistanceInches"].toDouble();

    return true;
}
