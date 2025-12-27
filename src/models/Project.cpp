#include "Project.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>

// ============================================================================
// QuoteRates
// ============================================================================

QJsonObject QuoteRates::toJson() const
{
    QJsonObject json;
    json["materialRatePerFt"] = materialRatePerFt;
    json["laborRatePerFt"] = laborRatePerFt;
    json["markupPercent"] = markupPercent;
    return json;
}

QuoteRates QuoteRates::fromJson(const QJsonObject& json)
{
    QuoteRates rates;
    rates.materialRatePerFt = json["materialRatePerFt"].toDouble(0.0);
    rates.laborRatePerFt = json["laborRatePerFt"].toDouble(0.0);
    rates.markupPercent = json["markupPercent"].toDouble(0.0);
    return rates;
}

// ============================================================================
// Project
// ============================================================================

const QString Project::FILE_EXTENSION = ".takeoff.json";
const QString Project::FILE_FILTER = "Takeoff Project (*.takeoff.json);;All Files (*)";

Project::Project()
    : m_pages()
    , m_measurements()
    , m_quoteRates()
    , m_lastError()
    , m_defaultCalibration()
{
}

// ============================================================================
// Pages
// ============================================================================

const QVector<Page>& Project::pages() const
{
    return m_pages;
}

QVector<Page>& Project::pages()
{
    return m_pages;
}

void Project::addPage(const Page& page)
{
    m_pages.append(page);
}

void Project::removePage(const QString& pageId)
{
    // Remove all measurements on this page first
    for (int i = m_measurements.size() - 1; i >= 0; --i) {
        if (m_measurements[i].pageId() == pageId) {
            m_measurements.removeAt(i);
        }
    }

    // Remove the page
    for (int i = 0; i < m_pages.size(); ++i) {
        if (m_pages[i].id() == pageId) {
            m_pages.removeAt(i);
            return;
        }
    }
}

Page* Project::findPage(const QString& pageId)
{
    for (int i = 0; i < m_pages.size(); ++i) {
        if (m_pages[i].id() == pageId) {
            return &m_pages[i];
        }
    }
    return nullptr;
}

const Page* Project::findPage(const QString& pageId) const
{
    for (int i = 0; i < m_pages.size(); ++i) {
        if (m_pages[i].id() == pageId) {
            return &m_pages[i];
        }
    }
    return nullptr;
}

Page* Project::pageAt(int index)
{
    if (index >= 0 && index < m_pages.size()) {
        return &m_pages[index];
    }
    return nullptr;
}

const Page* Project::pageAt(int index) const
{
    if (index >= 0 && index < m_pages.size()) {
        return &m_pages[index];
    }
    return nullptr;
}

int Project::pageIndex(const QString& pageId) const
{
    for (int i = 0; i < m_pages.size(); ++i) {
        if (m_pages[i].id() == pageId) {
            return i;
        }
    }
    return -1;
}

// ============================================================================
// Measurements
// ============================================================================

const QVector<Measurement>& Project::measurements() const
{
    return m_measurements;
}

QVector<Measurement>& Project::measurements()
{
    return m_measurements;
}

QVector<Measurement> Project::measurementsForPage(const QString& pageId) const
{
    QVector<Measurement> result;
    for (const Measurement& m : m_measurements) {
        if (m.pageId() == pageId) {
            result.append(m);
        }
    }
    return result;
}

void Project::setMeasurements(const QVector<Measurement>& measurements)
{
    m_measurements = measurements;
}

void Project::addMeasurement(const Measurement& measurement)
{
    m_measurements.append(measurement);
}

void Project::removeMeasurement(int id)
{
    for (int i = 0; i < m_measurements.size(); ++i) {
        if (m_measurements[i].id() == id) {
            m_measurements.removeAt(i);
            return;
        }
    }
}

Measurement* Project::findMeasurement(int id)
{
    for (int i = 0; i < m_measurements.size(); ++i) {
        if (m_measurements[i].id() == id) {
            return &m_measurements[i];
        }
    }
    return nullptr;
}

const Measurement* Project::findMeasurement(int id) const
{
    for (int i = 0; i < m_measurements.size(); ++i) {
        if (m_measurements[i].id() == id) {
            return &m_measurements[i];
        }
    }
    return nullptr;
}

int Project::nextMeasurementId() const
{
    int maxId = 0;
    for (const Measurement& m : m_measurements) {
        if (m.id() > maxId) {
            maxId = m.id();
        }
    }
    return maxId + 1;
}

// ============================================================================
// Quote Rates
// ============================================================================

const QuoteRates& Project::quoteRates() const
{
    return m_quoteRates;
}

QuoteRates& Project::quoteRates()
{
    return m_quoteRates;
}

void Project::setQuoteRates(const QuoteRates& rates)
{
    m_quoteRates = rates;
}

// ============================================================================
// Legacy Accessors
// ============================================================================

QString Project::imagePath() const
{
    // Return the first image page's path
    for (const Page& page : m_pages) {
        if (page.type() == PageType::Image) {
            return page.sourcePath();
        }
    }
    // Or just return the first page's source path
    if (!m_pages.isEmpty()) {
        return m_pages.first().sourcePath();
    }
    return QString();
}

const Calibration& Project::calibration() const
{
    // Return the first page's calibration
    if (!m_pages.isEmpty()) {
        return m_pages.first().calibration();
    }
    return m_defaultCalibration;
}

Calibration& Project::calibration()
{
    // Return the first page's calibration
    if (!m_pages.isEmpty()) {
        return m_pages.first().calibration();
    }
    return m_defaultCalibration;
}

// ============================================================================
// Project Management
// ============================================================================

void Project::clear()
{
    m_pages.clear();
    m_measurements.clear();
    m_quoteRates = QuoteRates();
    m_defaultCalibration.reset();
    m_lastError.clear();
}

bool Project::saveToJson(const QString& filePath) const
{
    QJsonObject root;

    // Version 2 format
    root["version"] = CURRENT_VERSION;

    // Pages array
    QJsonArray pagesArray;
    for (const Page& page : m_pages) {
        pagesArray.append(page.toJson());
    }
    root["pages"] = pagesArray;

    // Quote rates
    root["quoteRates"] = m_quoteRates.toJson();

    // Measurements array
    QJsonArray measurementsArray;
    for (const Measurement& m : m_measurements) {
        measurementsArray.append(m.toJson());
    }
    root["measurements"] = measurementsArray;

    // Write to file
    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file for writing: %1").arg(file.errorString());
        return false;
    }

    // Pretty-printed JSON
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    m_lastError.clear();
    return true;
}

bool Project::loadFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Cannot open file for reading: %1").arg(file.errorString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        m_lastError = QString("JSON parse error: %1").arg(parseError.errorString());
        return false;
    }

    if (!doc.isObject()) {
        m_lastError = "Invalid project file: root is not an object";
        return false;
    }

    QJsonObject root = doc.object();

    // Check version
    int version = root["version"].toInt(1);
    if (version > CURRENT_VERSION) {
        m_lastError = QString("Project file version %1 is not supported (max: %2)")
            .arg(version).arg(CURRENT_VERSION);
        return false;
    }

    // Clear existing data
    clear();

    // Handle version 1 (legacy single-image format)
    if (version == 1) {
        migrateFromVersion1(root);
        return m_lastError.isEmpty();
    }

    // Version 2+ format: load pages array
    QJsonArray pagesArray = root["pages"].toArray();
    for (const QJsonValue& val : pagesArray) {
        m_pages.append(Page::fromJson(val.toObject()));
    }

    // Quote rates (with defaults for backwards compatibility)
    if (root.contains("quoteRates")) {
        m_quoteRates = QuoteRates::fromJson(root["quoteRates"].toObject());
    }

    // Measurements
    QJsonArray measurementsArray = root["measurements"].toArray();
    for (const QJsonValue& val : measurementsArray) {
        m_measurements.append(Measurement::fromJson(val.toObject()));
    }

    m_lastError.clear();
    return true;
}

void Project::migrateFromVersion1(const QJsonObject& root)
{
    // Create a single Image page from old format
    QString imagePath = root["imagePath"].toString();
    
    if (!imagePath.isEmpty()) {
        Page page = Page::createImagePage(imagePath);
        
        // Load calibration into the page
        if (root.contains("calibration")) {
            page.calibration().fromJson(root["calibration"].toObject());
        }
        
        m_pages.append(page);
        
        // Load measurements and assign them to this page
        QJsonArray measurementsArray = root["measurements"].toArray();
        for (const QJsonValue& val : measurementsArray) {
            Measurement m = Measurement::fromJson(val.toObject());
            // Set pageId to the new page if not already set
            if (m.pageId().isEmpty()) {
                m.setPageId(page.id());
            }
            m_measurements.append(m);
        }
    }

    // Quote rates
    if (root.contains("quoteRates")) {
        m_quoteRates = QuoteRates::fromJson(root["quoteRates"].toObject());
    }

    m_lastError.clear();
}

QString Project::lastError() const
{
    return m_lastError;
}
