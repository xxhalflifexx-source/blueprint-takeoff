#include "Page.h"
#include <QFileInfo>
#include <QUuid>

Page::Page()
    : m_id(QUuid::createUuid().toString(QUuid::WithoutBraces))
    , m_type(Page::Image)
    , m_sourcePath()
    , m_pdfPageIndex(0)
    , m_pdfTotalPages(0)
    , m_displayName()
    , m_calibration()
{
}

Page Page::createImagePage(const QString& sourcePath)
{
    Page page;
    page.m_type = Page::Image;
    page.m_sourcePath = sourcePath;
    return page;
}

Page Page::createPdfPage(const QString& sourcePath, int pageIndex, int totalPages)
{
    Page page;
    page.m_type = Page::Pdf;
    page.m_sourcePath = sourcePath;
    page.m_pdfPageIndex = pageIndex;
    page.m_pdfTotalPages = totalPages;
    return page;
}

// ============================================================================
// Accessors
// ============================================================================

QString Page::id() const
{
    return m_id;
}

Page::Type Page::type() const
{
    return m_type;
}

QString Page::sourcePath() const
{
    return m_sourcePath;
}

int Page::pdfPageIndex() const
{
    return m_pdfPageIndex;
}

int Page::pdfTotalPages() const
{
    return m_pdfTotalPages;
}

QString Page::displayName() const
{
    return m_displayName;
}

const Calibration& Page::calibration() const
{
    return m_calibration;
}

Calibration& Page::calibration()
{
    return m_calibration;
}

// ============================================================================
// Setters
// ============================================================================

void Page::setId(const QString& id)
{
    m_id = id;
}

void Page::setType(Page::Type type)
{
    m_type = type;
}

void Page::setSourcePath(const QString& path)
{
    m_sourcePath = path;
}

void Page::setPdfPageIndex(int index)
{
    m_pdfPageIndex = index;
}

void Page::setPdfTotalPages(int total)
{
    m_pdfTotalPages = total;
}

void Page::setDisplayName(const QString& name)
{
    m_displayName = name;
}

void Page::setCalibration(const Calibration& calibration)
{
    m_calibration = calibration;
}

// ============================================================================
// Display
// ============================================================================

QString Page::listDisplayString() const
{
    QFileInfo fi(m_sourcePath);
    QString fileName = fi.fileName();
    
    if (!m_displayName.isEmpty()) {
        return m_displayName;
    }
    
    if (m_type == Page::Pdf) {
        return QString("PDF: %1 (%2/%3)")
            .arg(fileName)
            .arg(m_pdfPageIndex + 1)  // 1-based for display
            .arg(m_pdfTotalPages);
    } else {
        return QString("IMG: %1").arg(fileName);
    }
}

QString Page::typeString() const
{
    switch (m_type) {
        case Page::Image: return "Image";
        case Page::Pdf:   return "Pdf";
        default:          return "Image";
    }
}

Page::Type Page::typeFromString(const QString& str)
{
    if (str == "Pdf") return Page::Pdf;
    return Page::Image;
}

// ============================================================================
// JSON Serialization
// ============================================================================

QJsonObject Page::toJson() const
{
    QJsonObject json;
    json["id"] = m_id;
    json["type"] = typeString();
    json["sourcePath"] = m_sourcePath;
    json["pdfPageIndex"] = m_pdfPageIndex;
    json["pdfTotalPages"] = m_pdfTotalPages;
    json["displayName"] = m_displayName;
    json["calibration"] = m_calibration.toJson();
    return json;
}

Page Page::fromJson(const QJsonObject& json)
{
    Page page;
    page.m_id = json["id"].toString();
    if (page.m_id.isEmpty()) {
        page.m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    page.m_type = typeFromString(json["type"].toString());
    page.m_sourcePath = json["sourcePath"].toString();
    page.m_pdfPageIndex = json["pdfPageIndex"].toInt(0);
    page.m_pdfTotalPages = json["pdfTotalPages"].toInt(0);
    page.m_displayName = json["displayName"].toString();
    
    if (json.contains("calibration")) {
        page.m_calibration.fromJson(json["calibration"].toObject());
    }
    
    return page;
}

