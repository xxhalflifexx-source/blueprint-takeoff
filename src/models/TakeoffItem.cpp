#include "TakeoffItem.h"

TakeoffItem::TakeoffItem()
    : m_id(-1)
    , m_kind(Line)
    , m_lengthInches(0.0)
    , m_qty(1)
    , m_shapeId(-1)
{
}

TakeoffItem::TakeoffItem(Kind kind, const QVector<QPointF>& points, double lengthInches)
    : m_id(-1)
    , m_kind(kind)
    , m_points(points)
    , m_lengthInches(lengthInches)
    , m_qty(1)
    , m_shapeId(-1)
{
}

double TakeoffItem::weightLb(double wLbPerFt) const
{
    if (wLbPerFt <= 0.0) return 0.0;
    return totalLengthFeet() * wLbPerFt;
}

double TakeoffItem::materialCost(double wLbPerFt, double pricePerLb) const
{
    return weightLb(wLbPerFt) * pricePerLb;
}

QString TakeoffItem::displayString() const
{
    QString result = kindString();
    
    if (!m_designation.isEmpty()) {
        result += QString(" - %1").arg(m_designation);
    }
    
    result += QString(" (%.2f ft").arg(lengthFeet());
    
    if (m_qty > 1) {
        result += QString(" x%1").arg(m_qty);
    }
    
    result += ")";
    
    return result;
}

QString TakeoffItem::kindString() const
{
    switch (m_kind) {
        case Line: return "Line";
        case Polyline: return "Polyline";
        default: return "Unknown";
    }
}

