#include "PdfRenderer.h"

#ifdef HAS_QT_PDF
#include <QPdfDocument>
#endif

#include <QSizeF>

PdfRenderer::PdfRenderer()
#ifdef HAS_QT_PDF
    : m_document(std::make_unique<QPdfDocument>())
#endif
{
}

PdfRenderer::~PdfRenderer()
{
    close();
}

bool PdfRenderer::isAvailable()
{
#ifdef HAS_QT_PDF
    return true;
#else
    return false;
#endif
}

bool PdfRenderer::openPdf(const QString& path)
{
#ifdef HAS_QT_PDF
    close();
    
    QPdfDocument::Error error = m_document->load(path);
    
    if (error != QPdfDocument::Error::None) {
        switch (error) {
            case QPdfDocument::Error::FileNotFound:
                m_lastError = "PDF file not found: " + path;
                break;
            case QPdfDocument::Error::InvalidFileFormat:
                m_lastError = "Invalid PDF file format";
                break;
            case QPdfDocument::Error::IncorrectPassword:
                m_lastError = "PDF is password protected";
                break;
            case QPdfDocument::Error::UnsupportedSecurityScheme:
                m_lastError = "Unsupported PDF security scheme";
                break;
            default:
                m_lastError = "Unknown error opening PDF";
                break;
        }
        return false;
    }
    
    m_currentPath = path;
    m_lastError.clear();
    return true;
#else
    Q_UNUSED(path);
    m_lastError = "PDF support is not available. Qt PDF module is not installed.";
    return false;
#endif
}

bool PdfRenderer::isOpen() const
{
#ifdef HAS_QT_PDF
    return m_document->status() == QPdfDocument::Status::Ready;
#else
    return false;
#endif
}

int PdfRenderer::pageCount() const
{
#ifdef HAS_QT_PDF
    if (!isOpen()) {
        return 0;
    }
    return m_document->pageCount();
#else
    return 0;
#endif
}

QString PdfRenderer::currentPath() const
{
    return m_currentPath;
}

QImage PdfRenderer::renderPage(int pageIndex, double dpi) const
{
#ifdef HAS_QT_PDF
    if (!isOpen()) {
        m_lastError = "No PDF document loaded";
        return QImage();
    }
    
    if (pageIndex < 0 || pageIndex >= m_document->pageCount()) {
        m_lastError = QString("Invalid page index: %1").arg(pageIndex);
        return QImage();
    }
    
    // Get page size in points (1/72 inch)
    QSizeF pageSizePoints = m_document->pagePointSize(pageIndex);
    
    // Calculate pixel size based on DPI
    // points * (dpi / 72) = pixels
    double scale = dpi / 72.0;
    QSize pixelSize(
        static_cast<int>(pageSizePoints.width() * scale),
        static_cast<int>(pageSizePoints.height() * scale)
    );
    
    // Render the page
    QImage image = m_document->render(pageIndex, pixelSize);
    
    if (image.isNull()) {
        m_lastError = "Failed to render PDF page";
        return QImage();
    }
    
    m_lastError.clear();
    return image;
#else
    Q_UNUSED(pageIndex);
    Q_UNUSED(dpi);
    m_lastError = "PDF support is not available";
    return QImage();
#endif
}

QSizeF PdfRenderer::pageSize(int pageIndex) const
{
#ifdef HAS_QT_PDF
    if (!isOpen() || pageIndex < 0 || pageIndex >= m_document->pageCount()) {
        return QSizeF();
    }
    return m_document->pagePointSize(pageIndex);
#else
    Q_UNUSED(pageIndex);
    return QSizeF();
#endif
}

void PdfRenderer::close()
{
#ifdef HAS_QT_PDF
    m_document->close();
#endif
    m_currentPath.clear();
}

QString PdfRenderer::lastError() const
{
    return m_lastError;
}
