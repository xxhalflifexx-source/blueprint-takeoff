#include "Measurement.h"

Measurement::Measurement()
    : m_id(0)
    , m_pageId()
    , m_type(MeasurementType::Line)
    , m_lengthInches(0.0)
    , m_name()
    , m_notes()
    , m_category(Category::Misc)
    , m_materialType(MaterialType::Other)
    , m_size()
    , m_laborClass(LaborClass::ShopFab)
{
}

Measurement::Measurement(int id, MeasurementType type, const QVector<QPointF>& points, double lengthInches)
    : m_id(id)
    , m_pageId()
    , m_type(type)
    , m_points(points)
    , m_lengthInches(lengthInches)
    , m_name()
    , m_notes()
    , m_category(Category::Misc)
    , m_materialType(MaterialType::Other)
    , m_size()
    , m_laborClass(LaborClass::ShopFab)
{
}

// ============================================================================
// Basic Accessors
// ============================================================================

int Measurement::id() const
{
    return m_id;
}

QString Measurement::pageId() const
{
    return m_pageId;
}

MeasurementType Measurement::type() const
{
    return m_type;
}

const QVector<QPointF>& Measurement::points() const
{
    return m_points;
}

double Measurement::lengthInches() const
{
    return m_lengthInches;
}

double Measurement::lengthFeet() const
{
    return m_lengthInches / 12.0;
}

QString Measurement::name() const
{
    return m_name;
}

QString Measurement::notes() const
{
    return m_notes;
}

// ============================================================================
// Tagging Accessors
// ============================================================================

Category Measurement::category() const
{
    return m_category;
}

MaterialType Measurement::materialType() const
{
    return m_materialType;
}

QString Measurement::size() const
{
    return m_size;
}

LaborClass Measurement::laborClass() const
{
    return m_laborClass;
}

// ============================================================================
// Basic Setters
// ============================================================================

void Measurement::setId(int id)
{
    m_id = id;
}

void Measurement::setPageId(const QString& pageId)
{
    m_pageId = pageId;
}

void Measurement::setType(MeasurementType type)
{
    m_type = type;
}

void Measurement::setPoints(const QVector<QPointF>& points)
{
    m_points = points;
}

void Measurement::setLengthInches(double length)
{
    m_lengthInches = length;
}

void Measurement::setName(const QString& name)
{
    m_name = name;
}

void Measurement::setNotes(const QString& notes)
{
    m_notes = notes;
}

// ============================================================================
// Tagging Setters
// ============================================================================

void Measurement::setCategory(Category category)
{
    m_category = category;
}

void Measurement::setMaterialType(MaterialType materialType)
{
    m_materialType = materialType;
}

void Measurement::setSize(const QString& size)
{
    m_size = size;
}

void Measurement::setLaborClass(LaborClass laborClass)
{
    m_laborClass = laborClass;
}

// ============================================================================
// Display String
// ============================================================================

QString Measurement::displayString() const
{
    QString base = QString("%1: %2 in")
        .arg(typeString())
        .arg(m_lengthInches, 0, 'f', 2);
    
    if (!m_name.isEmpty()) {
        return QString("%1 - %2").arg(m_name, base);
    }
    return base;
}

// ============================================================================
// MeasurementType Conversions
// ============================================================================

QString Measurement::typeString() const
{
    switch (m_type) {
        case MeasurementType::Line:
            return "Line";
        case MeasurementType::Polyline:
            return "Polyline";
        default:
            return "Unknown";
    }
}

MeasurementType Measurement::typeFromString(const QString& str)
{
    if (str == "Polyline") {
        return MeasurementType::Polyline;
    }
    return MeasurementType::Line;  // Default
}

// ============================================================================
// Category Conversions
// ============================================================================

QString Measurement::categoryString() const
{
    switch (m_category) {
        case Category::Handrail: return "Handrail";
        case Category::Stairs:   return "Stairs";
        case Category::Platform: return "Platform";
        case Category::Misc:     return "Misc";
        default:                 return "Misc";
    }
}

Category Measurement::categoryFromString(const QString& str)
{
    if (str == "Handrail") return Category::Handrail;
    if (str == "Stairs")   return Category::Stairs;
    if (str == "Platform") return Category::Platform;
    return Category::Misc;  // Default
}

QStringList Measurement::categoryStrings()
{
    return {"Handrail", "Stairs", "Platform", "Misc"};
}

// ============================================================================
// MaterialType Conversions
// ============================================================================

QString Measurement::materialTypeString() const
{
    switch (m_materialType) {
        case MaterialType::Tube:    return "Tube";
        case MaterialType::Angle:   return "Angle";
        case MaterialType::Channel: return "Channel";
        case MaterialType::FlatBar: return "FlatBar";
        case MaterialType::Plate:   return "Plate";
        case MaterialType::Other:   return "Other";
        default:                    return "Other";
    }
}

MaterialType Measurement::materialTypeFromString(const QString& str)
{
    if (str == "Tube")    return MaterialType::Tube;
    if (str == "Angle")   return MaterialType::Angle;
    if (str == "Channel") return MaterialType::Channel;
    if (str == "FlatBar") return MaterialType::FlatBar;
    if (str == "Plate")   return MaterialType::Plate;
    return MaterialType::Other;  // Default
}

QStringList Measurement::materialTypeStrings()
{
    return {"Tube", "Angle", "Channel", "FlatBar", "Plate", "Other"};
}

// ============================================================================
// LaborClass Conversions
// ============================================================================

QString Measurement::laborClassString() const
{
    switch (m_laborClass) {
        case LaborClass::ShopFab:      return "ShopFab";
        case LaborClass::FieldInstall: return "FieldInstall";
        case LaborClass::FieldWeld:    return "FieldWeld";
        default:                       return "ShopFab";
    }
}

LaborClass Measurement::laborClassFromString(const QString& str)
{
    if (str == "ShopFab")      return LaborClass::ShopFab;
    if (str == "FieldInstall") return LaborClass::FieldInstall;
    if (str == "FieldWeld")    return LaborClass::FieldWeld;
    return LaborClass::ShopFab;  // Default
}

QStringList Measurement::laborClassStrings()
{
    return {"ShopFab", "FieldInstall", "FieldWeld"};
}

// ============================================================================
// JSON Serialization
// ============================================================================

QJsonObject Measurement::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["pageId"] = m_pageId;
    json["type"] = typeString();
    json["lengthInches"] = m_lengthInches;
    json["name"] = m_name;
    json["notes"] = m_notes;

    // Tagging fields
    json["category"] = categoryString();
    json["materialType"] = materialTypeString();
    json["size"] = m_size;
    json["laborClass"] = laborClassString();

    // Serialize points array
    QJsonArray pointsArray;
    for (const QPointF& pt : m_points) {
        QJsonObject ptObj;
        ptObj["x"] = pt.x();
        ptObj["y"] = pt.y();
        pointsArray.append(ptObj);
    }
    json["points"] = pointsArray;

    return json;
}

Measurement Measurement::fromJson(const QJsonObject& json)
{
    Measurement m;
    
    m.m_id = json["id"].toInt();
    m.m_pageId = json["pageId"].toString();  // Empty string if not present (backwards compat)
    m.m_type = typeFromString(json["type"].toString());
    m.m_lengthInches = json["lengthInches"].toDouble();
    m.m_name = json["name"].toString();
    m.m_notes = json["notes"].toString();

    // Tagging fields with defaults for backwards compatibility
    m.m_category = categoryFromString(json["category"].toString("Misc"));
    m.m_materialType = materialTypeFromString(json["materialType"].toString("Other"));
    m.m_size = json["size"].toString("");
    m.m_laborClass = laborClassFromString(json["laborClass"].toString("ShopFab"));

    // Deserialize points array
    QJsonArray pointsArray = json["points"].toArray();
    for (const QJsonValue& val : pointsArray) {
        QJsonObject ptObj = val.toObject();
        m.m_points.append(QPointF(ptObj["x"].toDouble(), ptObj["y"].toDouble()));
    }

    return m;
}
