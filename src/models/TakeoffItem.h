#ifndef TAKEOFFITEM_H
#define TAKEOFFITEM_H

#include <QString>
#include <QVector>
#include <QPointF>

/**
 * @brief Represents a takeoff measurement item with material assignment.
 * 
 * TakeoffItem replaces the old Measurement class with a streamlined model
 * focused on shape designation, quantity, and weight/cost calculations.
 */
class TakeoffItem
{
public:
    enum Kind {
        Line,
        Polyline
    };

    TakeoffItem();
    TakeoffItem(Kind kind, const QVector<QPointF>& points, double lengthInches);

    // Identity
    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString pageId() const { return m_pageId; }
    void setPageId(const QString& pageId) { m_pageId = pageId; }

    // Geometry
    Kind kind() const { return m_kind; }
    void setKind(Kind kind) { m_kind = kind; }

    QVector<QPointF> points() const { return m_points; }
    void setPoints(const QVector<QPointF>& points) { m_points = points; }

    double lengthInches() const { return m_lengthInches; }
    void setLengthInches(double inches) { m_lengthInches = inches; }

    // Computed length
    double lengthFeet() const { return m_lengthInches / 12.0; }

    // Quantity
    int qty() const { return m_qty; }
    void setQty(int qty) { m_qty = qty > 0 ? qty : 1; }

    // Material assignment
    int shapeId() const { return m_shapeId; }
    void setShapeId(int id) { m_shapeId = id; }

    QString designation() const { return m_designation; }
    void setDesignation(const QString& designation) { m_designation = designation; }

    // Notes
    QString notes() const { return m_notes; }
    void setNotes(const QString& notes) { m_notes = notes; }

    // Weight/cost calculations (require external data)
    double weightLb(double wLbPerFt) const;
    double materialCost(double wLbPerFt, double pricePerLb) const;

    // Total length considering quantity
    double totalLengthFeet() const { return lengthFeet() * m_qty; }
    double totalLengthInches() const { return m_lengthInches * m_qty; }

    // Display helpers
    QString displayString() const;
    QString kindString() const;

    // Check if material is assigned
    bool hasMaterial() const { return m_shapeId > 0 && !m_designation.isEmpty(); }

private:
    int m_id = -1;
    QString m_pageId;
    Kind m_kind = Line;
    QVector<QPointF> m_points;
    double m_lengthInches = 0.0;
    int m_qty = 1;
    int m_shapeId = -1;
    QString m_designation;
    QString m_notes;
};

#endif // TAKEOFFITEM_H

