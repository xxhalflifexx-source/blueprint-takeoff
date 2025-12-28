#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QVector>
#include <memory>

#include "TakeoffItem.h"
#include "Calibration.h"
#include "Page.h"
#include "../core/ProjectDatabase.h"

/**
 * @brief Represents a takeoff project with SQLite persistence.
 * 
 * The Project class manages in-memory data (pages, takeoff items) and
 * delegates all persistence to ProjectDatabase. Project files use the
 * .takeoff.db extension (SQLite database).
 */
class Project
{
public:
    /// File extension for project files
    static const QString FILE_EXTENSION;
    static const QString FILE_FILTER;

    Project();
    ~Project();

    // ========================================================================
    // Project File Operations
    // ========================================================================

    /**
     * @brief Create a new project database file.
     * @param filePath Path to .takeoff.db file
     * @return true if successful
     */
    bool create(const QString& filePath);

    /**
     * @brief Open an existing project database.
     * @param filePath Path to .takeoff.db file
     * @return true if successful
     */
    bool open(const QString& filePath);

    /**
     * @brief Close the current project.
     */
    void close();

    /**
     * @brief Check if a project is open.
     */
    bool isOpen() const;

    /**
     * @brief Get the current file path.
     */
    QString filePath() const;

    /**
     * @brief Get the project database for direct access.
     */
    ProjectDatabase* database() { return m_db.get(); }
    const ProjectDatabase* database() const { return m_db.get(); }

    // ========================================================================
    // Project Settings
    // ========================================================================

    QString name() const;
    void setName(const QString& name);

    double materialPricePerLb() const;
    void setMaterialPricePerLb(double pricePerLb);

    // ========================================================================
    // Pages
    // ========================================================================

    const QVector<Page>& pages() const;

    void addPage(const Page& page);
    void removePage(const QString& pageId);
    void updatePage(const Page& page);

    Page* findPage(const QString& pageId);
    const Page* findPage(const QString& pageId) const;

    Page* pageAt(int index);
    const Page* pageAt(int index) const;

    int pageIndex(const QString& pageId) const;

    /**
     * @brief Reload pages from database.
     */
    void reloadPages();

    // ========================================================================
    // Takeoff Items
    // ========================================================================

    const QVector<TakeoffItem>& takeoffItems() const;

    /**
     * @brief Get items for a specific page.
     */
    QVector<TakeoffItem> takeoffItemsForPage(const QString& pageId) const;

    /**
     * @brief Add a takeoff item. ID is assigned by database.
     * @return The assigned ID
     */
    int addTakeoffItem(TakeoffItem& item);

    /**
     * @brief Update an existing takeoff item.
     */
    void updateTakeoffItem(const TakeoffItem& item);

    /**
     * @brief Remove a takeoff item.
     */
    void removeTakeoffItem(int id);

    /**
     * @brief Find a takeoff item by ID.
     */
    TakeoffItem* findTakeoffItem(int id);
    const TakeoffItem* findTakeoffItem(int id) const;

    /**
     * @brief Reload items from database.
     */
    void reloadTakeoffItems();

    // ========================================================================
    // Shapes
    // ========================================================================

    /**
     * @brief Check if shapes are loaded.
     */
    bool hasShapes() const;

    /**
     * @brief Get shape count.
     */
    int shapeCount() const;

    /**
     * @brief Get all designations for autocomplete.
     */
    QStringList allDesignations() const;

    /**
     * @brief Get shape types for filtering.
     */
    QStringList shapeTypes() const;

    /**
     * @brief Search shapes.
     */
    QVector<ProjectDatabase::Shape> searchShapes(const QString& text, 
                                                  const QString& typeFilter = QString(),
                                                  int limit = 100) const;

    /**
     * @brief Get shape by ID.
     */
    ProjectDatabase::Shape getShape(int shapeId) const;

    /**
     * @brief Get shape by designation.
     */
    ProjectDatabase::Shape getShapeByDesignation(const QString& designation) const;

    /**
     * @brief Import shapes from CSV.
     * @return Number of shapes imported, or -1 on error
     */
    int importShapesFromCsv(const QString& csvPath);

    // ========================================================================
    // Error Handling
    // ========================================================================

    QString lastError() const;

private:
    std::unique_ptr<ProjectDatabase> m_db;
    QVector<Page> m_pages;
    QVector<TakeoffItem> m_takeoffItems;
    mutable QString m_lastError;
};

#endif // PROJECT_H
