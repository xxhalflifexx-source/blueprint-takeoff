#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QVector>
#include <QJsonObject>

#include "Measurement.h"
#include "Calibration.h"
#include "Page.h"

/**
 * @brief Quote rate settings for pricing calculations.
 */
struct QuoteRates
{
    double materialRatePerFt = 0.0;   // $/ft for material
    double laborRatePerFt = 0.0;      // $/ft for labor
    double markupPercent = 0.0;       // Markup percentage

    QJsonObject toJson() const;
    static QuoteRates fromJson(const QJsonObject& json);
};

/**
 * @brief Represents a takeoff project containing pages, calibration, and measurements.
 * 
 * Provides serialization to/from JSON for project save/load functionality.
 * Project files use the .takeoff.json extension.
 * 
 * Version 2 format supports multiple pages (image or PDF).
 * Version 1 format (single image) is supported for backwards compatibility.
 */
class Project
{
public:
    /// File extension for project files
    static const QString FILE_EXTENSION;
    static const QString FILE_FILTER;

    /// Current JSON format version
    static const int CURRENT_VERSION = 2;

    Project();

    // ========================================================================
    // Pages
    // ========================================================================

    /**
     * @brief Get all pages.
     * @return Vector of pages
     */
    const QVector<Page>& pages() const;
    QVector<Page>& pages();

    /**
     * @brief Add a page to the project.
     * @param page Page to add
     */
    void addPage(const Page& page);

    /**
     * @brief Remove a page by ID.
     * @param pageId Page ID to remove
     * 
     * Note: This also removes all measurements on that page.
     */
    void removePage(const QString& pageId);

    /**
     * @brief Find a page by ID.
     * @param pageId Page ID
     * @return Pointer to page, or nullptr if not found
     */
    Page* findPage(const QString& pageId);
    const Page* findPage(const QString& pageId) const;

    /**
     * @brief Get page by index.
     * @param index Page index
     * @return Pointer to page, or nullptr if index invalid
     */
    Page* pageAt(int index);
    const Page* pageAt(int index) const;

    /**
     * @brief Get index of page by ID.
     * @param pageId Page ID
     * @return Index, or -1 if not found
     */
    int pageIndex(const QString& pageId) const;

    // ========================================================================
    // Measurements
    // ========================================================================

    /**
     * @brief Get all measurements.
     * @return Vector of all measurements
     */
    const QVector<Measurement>& measurements() const;
    QVector<Measurement>& measurements();

    /**
     * @brief Get measurements for a specific page.
     * @param pageId Page ID
     * @return Vector of measurements on that page
     */
    QVector<Measurement> measurementsForPage(const QString& pageId) const;

    void setMeasurements(const QVector<Measurement>& measurements);
    void addMeasurement(const Measurement& measurement);
    void removeMeasurement(int id);
    Measurement* findMeasurement(int id);
    const Measurement* findMeasurement(int id) const;
    int nextMeasurementId() const;

    // ========================================================================
    // Quote rates
    // ========================================================================

    const QuoteRates& quoteRates() const;
    QuoteRates& quoteRates();
    void setQuoteRates(const QuoteRates& rates);

    // ========================================================================
    // Legacy accessors (for backwards compatibility during migration)
    // ========================================================================

    /**
     * @brief Get the primary image path (first image page).
     * @return Image path, or empty if no image pages
     * @deprecated Use pages() instead
     */
    QString imagePath() const;

    /**
     * @brief Get the primary calibration (first page's calibration).
     * @return Calibration, or default if no pages
     * @deprecated Use page-specific calibration instead
     */
    const Calibration& calibration() const;
    Calibration& calibration();

    // ========================================================================
    // Project management
    // ========================================================================

    /**
     * @brief Clear all project data.
     */
    void clear();

    /**
     * @brief Save project to a JSON file.
     * @param filePath Path to save to
     * @return true if successful, false on error
     */
    bool saveToJson(const QString& filePath) const;

    /**
     * @brief Load project from a JSON file.
     * @param filePath Path to load from
     * @return true if successful, false on error
     */
    bool loadFromJson(const QString& filePath);

    /**
     * @brief Get the last error message.
     * @return Error description string
     */
    QString lastError() const;

private:
    QVector<Page> m_pages;
    QVector<Measurement> m_measurements;
    QuoteRates m_quoteRates;
    mutable QString m_lastError;

    // For legacy calibration() accessor when no pages exist
    mutable Calibration m_defaultCalibration;

    /**
     * @brief Migrate version 1 project format to version 2.
     * @param root JSON root object
     */
    void migrateFromVersion1(const QJsonObject& root);
};

#endif // PROJECT_H
