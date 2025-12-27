#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include <QString>
#include <QImage>
#include <memory>

#ifdef HAS_QT_PDF
class QPdfDocument;
#endif

/**
 * @brief Renders PDF pages to QImage using Qt's PDF module.
 * 
 * If Qt PDF module is not available, all operations will fail gracefully.
 */
class PdfRenderer
{
public:
    PdfRenderer();
    ~PdfRenderer();

    /**
     * @brief Check if PDF support is available.
     * @return true if Qt PDF module is available
     */
    static bool isAvailable();

    /**
     * @brief Open a PDF file.
     * @param path Path to the PDF file
     * @return true if opened successfully
     */
    bool openPdf(const QString& path);

    /**
     * @brief Check if a PDF is currently open.
     * @return true if a PDF is loaded
     */
    bool isOpen() const;

    /**
     * @brief Get the number of pages in the PDF.
     * @return Page count, or 0 if no PDF loaded
     */
    int pageCount() const;

    /**
     * @brief Get the current PDF file path.
     * @return File path, or empty if no PDF loaded
     */
    QString currentPath() const;

    /**
     * @brief Render a page to QImage.
     * @param pageIndex 0-based page index
     * @param dpi Resolution in dots per inch (default 150)
     * @return Rendered image, or null image on error
     */
    QImage renderPage(int pageIndex, double dpi = 150.0) const;

    /**
     * @brief Get the size of a page in points.
     * @param pageIndex 0-based page index
     * @return Page size in points (1/72 inch)
     */
    QSizeF pageSize(int pageIndex) const;

    /**
     * @brief Close the current PDF.
     */
    void close();

    /**
     * @brief Get the last error message.
     * @return Error description
     */
    QString lastError() const;

private:
#ifdef HAS_QT_PDF
    std::unique_ptr<QPdfDocument> m_document;
#endif
    QString m_currentPath;
    mutable QString m_lastError;
};

#endif // PDFRENDERER_H
