#ifndef PROJECTDATABASE_H
#define PROJECTDATABASE_H

#include <QString>
#include <QVector>
#include <QPointF>
#include <QSqlDatabase>

// Forward declarations
class TakeoffItem;
class Page;
struct ShapeRow;

/**
 * @brief Manages SQLite database for project persistence.
 * 
 * Handles all CRUD operations for pages, takeoff items, shapes, and project settings.
 * Each project is stored as a single .takeoff.db file.
 */
class ProjectDatabase
{
public:
    ProjectDatabase();
    ~ProjectDatabase();

    /**
     * @brief Create a new project database file.
     * @param path Path to the .takeoff.db file
     * @return true if successful
     */
    bool create(const QString& path);

    /**
     * @brief Open an existing project database.
     * @param path Path to the .takeoff.db file
     * @return true if successful
     */
    bool open(const QString& path);

    /**
     * @brief Close the database connection.
     */
    void close();

    /**
     * @brief Check if the database is open.
     */
    bool isOpen() const;

    /**
     * @brief Get the current database file path.
     */
    QString filePath() const;

    // =========================================================================
    // Project Settings
    // =========================================================================

    QString getProjectSetting(const QString& key, const QString& defaultValue = QString()) const;
    void setProjectSetting(const QString& key, const QString& value);

    double getMaterialPricePerLb() const;
    void setMaterialPricePerLb(double pricePerLb);

    QString getProjectName() const;
    void setProjectName(const QString& name);

    // =========================================================================
    // Pages
    // =========================================================================

    bool insertPage(const Page& page);
    bool updatePage(const Page& page);
    bool deletePage(const QString& pageId);
    Page getPage(const QString& pageId) const;
    QVector<Page> getAllPages() const;

    // =========================================================================
    // Takeoff Items
    // =========================================================================

    int insertTakeoffItem(const TakeoffItem& item);  // Returns new ID
    bool updateTakeoffItem(const TakeoffItem& item);
    bool deleteTakeoffItem(int itemId);
    TakeoffItem getTakeoffItem(int itemId) const;
    QVector<TakeoffItem> getTakeoffItemsForPage(const QString& pageId) const;
    QVector<TakeoffItem> getAllTakeoffItems() const;

    // =========================================================================
    // Shapes (AISC database)
    // =========================================================================

    int insertShape(const QString& designation, const QString& shapeType, double wLbPerFt);
    bool updateShape(int shapeId, const QString& designation, const QString& shapeType, double wLbPerFt);
    bool deleteShape(int shapeId);
    
    struct Shape {
        int id = -1;
        QString designation;
        QString shapeType;
        double wLbPerFt = 0.0;
    };
    
    Shape getShape(int shapeId) const;
    Shape getShapeByDesignation(const QString& designation) const;
    QVector<Shape> getAllShapes() const;
    QVector<Shape> searchShapes(const QString& searchText, const QString& typeFilter = QString(), int limit = 100) const;
    QStringList getAllDesignations() const;  // For autocomplete
    QStringList getShapeTypes() const;
    int getShapeCount() const;
    bool hasShapes() const;
    void clearShapes();

    /**
     * @brief Import shapes from CSV file.
     * @param filePath Path to CSV file
     * @return Number of shapes imported, or -1 on error
     */
    int importShapesFromCsv(const QString& filePath);

    /**
     * @brief Get the last error message.
     */
    QString lastError() const;

private:
    void createSchema();
    QString serializePoints(const QVector<QPointF>& points) const;
    QVector<QPointF> deserializePoints(const QString& json) const;

    QSqlDatabase m_db;
    QString m_connectionName;
    QString m_filePath;
    mutable QString m_lastError;
    bool m_isOpen;
};

#endif // PROJECTDATABASE_H

