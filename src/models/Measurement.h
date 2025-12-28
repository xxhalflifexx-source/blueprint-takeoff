#ifndef MEASUREMENT_H
#define MEASUREMENT_H

#include <QPointF>
#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

/**
 * @brief Type of measurement.
 */
enum class MeasurementType
{
    Line,       // Simple two-point line
    Polyline    // Multi-point connected line
};

/**
 * @brief Category for grouping measurements.
 */
enum class Category
{
    Handrail,
    Stairs,
    Platform,
    Misc
};

/**
 * @brief Material type for quote calculations.
 */
enum class MaterialType
{
    Tube,
    Angle,
    Channel,
    FlatBar,
    Plate,
    Other
};

/**
 * @brief Labor classification for pricing.
 */
enum class LaborClass
{
    ShopFab,
    FieldInstall,
    FieldWeld
};

/**
 * @brief Represents a single measurement on the blueprint.
 * 
 * Stores the points (in scene/pixel coordinates), the calculated
 * length in real-world inches, and tagging properties for quoting.
 * Each measurement belongs to exactly one Page (via pageId).
 */
class Measurement
{
public:
    Measurement();
    Measurement(int id, MeasurementType type, const QVector<QPointF>& points, double lengthInches);

    // Accessors - Basic
    int id() const;
    QString pageId() const;
    MeasurementType type() const;
    const QVector<QPointF>& points() const;
    double lengthInches() const;
    double lengthFeet() const;
    QString name() const;
    QString notes() const;

    // Accessors - Tagging
    Category category() const;
    MaterialType materialType() const;
    QString size() const;
    LaborClass laborClass() const;

    // Accessors - AISC Shape
    int shapeId() const;
    QString shapeLabel() const;

    // Setters - Basic
    void setId(int id);
    void setPageId(const QString& pageId);
    void setType(MeasurementType type);
    void setPoints(const QVector<QPointF>& points);
    void setLengthInches(double length);
    void setName(const QString& name);
    void setNotes(const QString& notes);

    // Setters - Tagging
    void setCategory(Category category);
    void setMaterialType(MaterialType materialType);
    void setSize(const QString& size);
    void setLaborClass(LaborClass laborClass);

    // Setters - AISC Shape
    void setShapeId(int id);
    void setShapeLabel(const QString& label);

    /**
     * @brief Get a display string for the measurement.
     * @return String like "Line: 24.50 in" or "Polyline: 48.25 in"
     */
    QString displayString() const;

    // Type string conversions
    QString typeString() const;
    static MeasurementType typeFromString(const QString& str);

    // Category string conversions
    QString categoryString() const;
    static Category categoryFromString(const QString& str);
    static QStringList categoryStrings();

    // MaterialType string conversions
    QString materialTypeString() const;
    static MaterialType materialTypeFromString(const QString& str);
    static QStringList materialTypeStrings();

    // LaborClass string conversions
    QString laborClassString() const;
    static LaborClass laborClassFromString(const QString& str);
    static QStringList laborClassStrings();

    /**
     * @brief Serialize measurement to JSON.
     * @return JSON object representation
     */
    QJsonObject toJson() const;

    /**
     * @brief Deserialize measurement from JSON.
     * @param json JSON object to read from
     * @return Measurement object (default if parsing fails)
     */
    static Measurement fromJson(const QJsonObject& json);

private:
    int m_id;
    QString m_pageId;  // ID of the page this measurement belongs to
    MeasurementType m_type;
    QVector<QPointF> m_points;
    double m_lengthInches;
    QString m_name;
    QString m_notes;

    // Tagging fields
    Category m_category;
    MaterialType m_materialType;
    QString m_size;
    LaborClass m_laborClass;

    // AISC Shape reference
    int m_shapeId;
    QString m_shapeLabel;
};

#endif // MEASUREMENT_H
