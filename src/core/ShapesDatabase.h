#ifndef SHAPESDATABASE_H
#define SHAPESDATABASE_H

#include <QString>
#include <QVector>
#include <QStringList>
#include <QSqlDatabase>

/**
 * @brief Lightweight row structure for displaying shapes in the picker.
 */
struct ShapeRow
{
    int id = -1;
    QString shapeKey;
    QString aiscLabel;
    QString ediName;
    QString shapeType;
    double weightPerFt = 0.0;  // "W" property (weight per foot)
    double depth = 0.0;        // "d" property
    double flangeWidth = 0.0;  // "bf" property
};

/**
 * @brief Manages the AISC shapes SQLite database.
 * 
 * Stores shapes imported from AISC spreadsheets (XLSX/CSV) in a local SQLite
 * database. Provides methods for importing data and querying shapes.
 * 
 * Database location: QStandardPaths::AppDataLocation/shapes.db
 * 
 * Schema:
 * - shapes(id, shape_key, aisc_label, edi_name, shape_type)
 * - shape_props(shape_id, prop_key, prop_value, prop_value_num)
 */
class ShapesDatabase
{
public:
    ShapesDatabase();
    ~ShapesDatabase();

    /**
     * @brief Open the database (creates if doesn't exist).
     * @return true if successful
     */
    bool open();

    /**
     * @brief Close the database connection.
     */
    void close();

    /**
     * @brief Check if the database is open.
     * @return true if connected
     */
    bool isOpen() const;

    /**
     * @brief Import shapes from an AISC XLSX spreadsheet.
     * @param filePath Path to the XLSX file
     * @return Number of shapes imported, or -1 on error
     */
    int importFromXlsx(const QString& filePath);

    /**
     * @brief Import shapes from a CSV file (fallback).
     * @param filePath Path to the CSV file
     * @return Number of shapes imported, or -1 on error
     */
    int importFromCsv(const QString& filePath);

    /**
     * @brief Query shapes with optional type filter and search text.
     * @param typeFilter Shape type filter (e.g., "W", "C", "L", "HSS") or empty for all
     * @param searchText Search text to filter by label/name
     * @param limit Maximum number of results
     * @return Vector of matching ShapeRow entries
     */
    QVector<ShapeRow> queryShapes(const QString& typeFilter = QString(),
                                   const QString& searchText = QString(),
                                   int limit = 100) const;

    /**
     * @brief Get a specific property value for a shape.
     * @param shapeId Shape ID
     * @param propKey Property key (e.g., "W", "d", "bf")
     * @return Numeric value or 0.0 if not found
     */
    double getShapeProperty(int shapeId, const QString& propKey) const;

    /**
     * @brief Get the AISC label for a shape.
     * @param shapeId Shape ID
     * @return AISC label or empty string if not found
     */
    QString getShapeLabel(int shapeId) const;

    /**
     * @brief Get available shape types in the database.
     * @return List of unique shape types (e.g., "W", "C", "L", "HSS")
     */
    QStringList getShapeTypes() const;

    /**
     * @brief Check if the database has any shapes.
     * @return true if shapes exist
     */
    bool hasShapes() const;

    /**
     * @brief Get the total number of shapes in the database.
     * @return Shape count
     */
    int shapeCount() const;

    /**
     * @brief Clear all shapes from the database.
     */
    void clearShapes();

    /**
     * @brief Get the last error message.
     * @return Error message string
     */
    QString lastError() const;

private:
    void createSchema();
    QString determineShapeType(const QString& label) const;
    QString findHeaderValue(const QStringList& headers, const QVector<QString>& row, 
                            const QStringList& possibleNames) const;

    QSqlDatabase m_db;
    QString m_connectionName;
    mutable QString m_lastError;
    bool m_isOpen;
};

#endif // SHAPESDATABASE_H

