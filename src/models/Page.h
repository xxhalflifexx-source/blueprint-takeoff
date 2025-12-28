#ifndef PAGE_H
#define PAGE_H

#include <QString>
#include <QJsonObject>
#include <QUuid>

#include "Calibration.h"

/**
 * @brief Represents a single page in a multi-page project.
 * 
 * Each page has its own source (image or PDF page), calibration,
 * and associated measurements.
 */
class Page
{
public:
    /**
     * @brief Type of page source.
     */
    enum Type {
        Image,  // PNG/JPG image file
        Pdf     // PDF file (single page)
    };

    Page();
    
    /**
     * @brief Create an image page.
     * @param sourcePath Path to the image file
     * @return New Page object
     */
    static Page createImagePage(const QString& sourcePath);
    
    /**
     * @brief Create a PDF page.
     * @param sourcePath Path to the PDF file
     * @param pageIndex 0-based page index
     * @param totalPages Total pages in the PDF
     * @return New Page object
     */
    static Page createPdfPage(const QString& sourcePath, int pageIndex, int totalPages);

    // Accessors
    QString id() const;
    Type type() const;
    QString sourcePath() const;
    int pdfPageIndex() const;
    int pdfTotalPages() const;
    QString displayName() const;
    const Calibration& calibration() const;
    Calibration& calibration();

    // Setters
    void setId(const QString& id);
    void setType(Type type);
    void setSourcePath(const QString& path);
    void setPdfPageIndex(int index);
    void setPdfTotalPages(int total);
    void setDisplayName(const QString& name);
    void setCalibration(const Calibration& calibration);

    /**
     * @brief Get display string for the page list.
     * @return String like "IMG: filename.jpg" or "PDF: file.pdf (3/12)"
     */
    QString listDisplayString() const;

    /**
     * @brief Get type as string.
     * @return "Image" or "Pdf"
     */
    QString typeString() const;
    
    /**
     * @brief Parse type from string.
     * @param str String representation
     * @return Page::Type
     */
    static Type typeFromString(const QString& str);

    /**
     * @brief Serialize to JSON.
     * @return JSON object
     */
    QJsonObject toJson() const;

    /**
     * @brief Deserialize from JSON.
     * @param json JSON object
     * @return Page object
     */
    static Page fromJson(const QJsonObject& json);

private:
    QString m_id;
    Type m_type;
    QString m_sourcePath;
    int m_pdfPageIndex;
    int m_pdfTotalPages;
    QString m_displayName;
    Calibration m_calibration;
};

#endif // PAGE_H

