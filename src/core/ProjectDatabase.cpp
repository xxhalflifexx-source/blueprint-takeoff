#include "ProjectDatabase.h"
#include "../models/TakeoffItem.h"
#include "../models/Page.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QUuid>
#include <QDebug>

ProjectDatabase::ProjectDatabase()
    : m_isOpen(false)
{
    // Generate unique connection name for this instance
    m_connectionName = QString("ProjectDB_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

ProjectDatabase::~ProjectDatabase()
{
    close();
}

bool ProjectDatabase::create(const QString& path)
{
    close();

    // Remove existing file if present
    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    m_filePath = path;
    m_isOpen = true;

    createSchema();

    // Set default project settings
    setProjectSetting("created_at", QDateTime::currentDateTime().toString(Qt::ISODate));
    setMaterialPricePerLb(0.50);  // Default $/lb

    return true;
}

bool ProjectDatabase::open(const QString& path)
{
    close();

    if (!QFile::exists(path)) {
        m_lastError = "File does not exist: " + path;
        return false;
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    m_filePath = path;
    m_isOpen = true;

    // Ensure schema exists (for older files)
    createSchema();

    return true;
}

void ProjectDatabase::close()
{
    if (m_isOpen) {
        m_db.close();
        m_isOpen = false;
        m_filePath.clear();
    }
    
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool ProjectDatabase::isOpen() const
{
    return m_isOpen;
}

QString ProjectDatabase::filePath() const
{
    return m_filePath;
}

void ProjectDatabase::createSchema()
{
    QSqlQuery query(m_db);

    // Project settings table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS project (
            key TEXT PRIMARY KEY,
            value TEXT
        )
    )");

    // Pages table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS pages (
            id TEXT PRIMARY KEY,
            type TEXT,
            source_path TEXT,
            pdf_page_index INTEGER,
            pdf_total_pages INTEGER,
            display_name TEXT,
            calibration_ppi REAL,
            calib_pt1_x REAL,
            calib_pt1_y REAL,
            calib_pt2_x REAL,
            calib_pt2_y REAL
        )
    )");

    // Shapes table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS shapes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            designation TEXT UNIQUE,
            shape_type TEXT,
            w_lb_per_ft REAL
        )
    )");

    // Takeoff items table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS takeoff_items (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            page_id TEXT REFERENCES pages(id),
            kind TEXT,
            points TEXT,
            length_in REAL,
            qty INTEGER DEFAULT 1,
            shape_id INTEGER REFERENCES shapes(id),
            designation TEXT,
            notes TEXT
        )
    )");

    // Create indexes
    query.exec("CREATE INDEX IF NOT EXISTS idx_shapes_type ON shapes(shape_type)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_shapes_designation ON shapes(designation)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_items_page ON takeoff_items(page_id)");
}

// =========================================================================
// Project Settings
// =========================================================================

QString ProjectDatabase::getProjectSetting(const QString& key, const QString& defaultValue) const
{
    if (!m_isOpen) return defaultValue;

    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM project WHERE key = ?");
    query.addBindValue(key);
    
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return defaultValue;
}

void ProjectDatabase::setProjectSetting(const QString& key, const QString& value)
{
    if (!m_isOpen) return;

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO project (key, value) VALUES (?, ?)");
    query.addBindValue(key);
    query.addBindValue(value);
    query.exec();
}

double ProjectDatabase::getMaterialPricePerLb() const
{
    QString val = getProjectSetting("material_price_per_lb", "0.50");
    return val.toDouble();
}

void ProjectDatabase::setMaterialPricePerLb(double pricePerLb)
{
    setProjectSetting("material_price_per_lb", QString::number(pricePerLb, 'f', 4));
}

QString ProjectDatabase::getProjectName() const
{
    return getProjectSetting("name", "Untitled Project");
}

void ProjectDatabase::setProjectName(const QString& name)
{
    setProjectSetting("name", name);
}

// =========================================================================
// Pages
// =========================================================================

bool ProjectDatabase::insertPage(const Page& page)
{
    if (!m_isOpen) return false;

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO pages (id, type, source_path, pdf_page_index, pdf_total_pages, 
                           display_name, calibration_ppi, calib_pt1_x, calib_pt1_y, 
                           calib_pt2_x, calib_pt2_y)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(page.id());
    query.addBindValue(page.type() == Page::Image ? "image" : "pdf");
    query.addBindValue(page.sourcePath());
    query.addBindValue(page.pdfPageIndex());
    query.addBindValue(page.pdfTotalPages());
    query.addBindValue(page.displayName());
    query.addBindValue(page.calibration().pixelsPerInch());
    query.addBindValue(page.calibration().calibrationPoint1().x());
    query.addBindValue(page.calibration().calibrationPoint1().y());
    query.addBindValue(page.calibration().calibrationPoint2().x());
    query.addBindValue(page.calibration().calibrationPoint2().y());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool ProjectDatabase::updatePage(const Page& page)
{
    if (!m_isOpen) return false;

    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE pages SET type = ?, source_path = ?, pdf_page_index = ?, 
                         pdf_total_pages = ?, display_name = ?, calibration_ppi = ?,
                         calib_pt1_x = ?, calib_pt1_y = ?, calib_pt2_x = ?, calib_pt2_y = ?
        WHERE id = ?
    )");
    
    query.addBindValue(page.type() == Page::Image ? "image" : "pdf");
    query.addBindValue(page.sourcePath());
    query.addBindValue(page.pdfPageIndex());
    query.addBindValue(page.pdfTotalPages());
    query.addBindValue(page.displayName());
    query.addBindValue(page.calibration().pixelsPerInch());
    query.addBindValue(page.calibration().calibrationPoint1().x());
    query.addBindValue(page.calibration().calibrationPoint1().y());
    query.addBindValue(page.calibration().calibrationPoint2().x());
    query.addBindValue(page.calibration().calibrationPoint2().y());
    query.addBindValue(page.id());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool ProjectDatabase::deletePage(const QString& pageId)
{
    if (!m_isOpen) return false;

    // First delete all takeoff items for this page
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM takeoff_items WHERE page_id = ?");
    query.addBindValue(pageId);
    query.exec();

    // Then delete the page
    query.prepare("DELETE FROM pages WHERE id = ?");
    query.addBindValue(pageId);
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

Page ProjectDatabase::getPage(const QString& pageId) const
{
    Page page;
    if (!m_isOpen) return page;

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM pages WHERE id = ?");
    query.addBindValue(pageId);
    
    if (query.exec() && query.next()) {
        page.setId(query.value("id").toString());
        page.setType(query.value("type").toString() == "image" ? Page::Image : Page::Pdf);
        page.setSourcePath(query.value("source_path").toString());
        page.setPdfPageIndex(query.value("pdf_page_index").toInt());
        page.setPdfTotalPages(query.value("pdf_total_pages").toInt());
        page.setDisplayName(query.value("display_name").toString());
        
        // Restore calibration
        Calibration cal;
        cal.setPixelsPerInch(query.value("calibration_ppi").toDouble());
        cal.setCalibrationPoints(
            QPointF(query.value("calib_pt1_x").toDouble(), query.value("calib_pt1_y").toDouble()),
            QPointF(query.value("calib_pt2_x").toDouble(), query.value("calib_pt2_y").toDouble())
        );
        page.setCalibration(cal);
    }
    return page;
}

QVector<Page> ProjectDatabase::getAllPages() const
{
    QVector<Page> pages;
    if (!m_isOpen) return pages;

    QSqlQuery query(m_db);
    query.exec("SELECT * FROM pages ORDER BY rowid");
    
    while (query.next()) {
        Page page;
        page.setId(query.value("id").toString());
        page.setType(query.value("type").toString() == "image" ? Page::Image : Page::Pdf);
        page.setSourcePath(query.value("source_path").toString());
        page.setPdfPageIndex(query.value("pdf_page_index").toInt());
        page.setPdfTotalPages(query.value("pdf_total_pages").toInt());
        page.setDisplayName(query.value("display_name").toString());
        
        // Restore calibration
        Calibration cal;
        cal.setPixelsPerInch(query.value("calibration_ppi").toDouble());
        cal.setCalibrationPoints(
            QPointF(query.value("calib_pt1_x").toDouble(), query.value("calib_pt1_y").toDouble()),
            QPointF(query.value("calib_pt2_x").toDouble(), query.value("calib_pt2_y").toDouble())
        );
        page.setCalibration(cal);
        
        pages.append(page);
    }
    return pages;
}

// =========================================================================
// Takeoff Items
// =========================================================================

QString ProjectDatabase::serializePoints(const QVector<QPointF>& points) const
{
    QJsonArray arr;
    for (const QPointF& pt : points) {
        QJsonObject obj;
        obj["x"] = pt.x();
        obj["y"] = pt.y();
        arr.append(obj);
    }
    return QJsonDocument(arr).toJson(QJsonDocument::Compact);
}

QVector<QPointF> ProjectDatabase::deserializePoints(const QString& json) const
{
    QVector<QPointF> points;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        for (const QJsonValue& val : arr) {
            QJsonObject obj = val.toObject();
            points.append(QPointF(obj["x"].toDouble(), obj["y"].toDouble()));
        }
    }
    return points;
}

int ProjectDatabase::insertTakeoffItem(const TakeoffItem& item)
{
    if (!m_isOpen) return -1;

    QSqlQuery query(m_db);
    query.prepare(R"(
        INSERT INTO takeoff_items (page_id, kind, points, length_in, qty, shape_id, designation, notes)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(item.pageId());
    query.addBindValue(item.kind() == TakeoffItem::Line ? "Line" : "Polyline");
    query.addBindValue(serializePoints(item.points()));
    query.addBindValue(item.lengthInches());
    query.addBindValue(item.qty());
    query.addBindValue(item.shapeId() > 0 ? item.shapeId() : QVariant());
    query.addBindValue(item.designation());
    query.addBindValue(item.notes());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return -1;
    }
    
    return query.lastInsertId().toInt();
}

bool ProjectDatabase::updateTakeoffItem(const TakeoffItem& item)
{
    if (!m_isOpen) return false;

    QSqlQuery query(m_db);
    query.prepare(R"(
        UPDATE takeoff_items SET page_id = ?, kind = ?, points = ?, length_in = ?, 
                                  qty = ?, shape_id = ?, designation = ?, notes = ?
        WHERE id = ?
    )");
    
    query.addBindValue(item.pageId());
    query.addBindValue(item.kind() == TakeoffItem::Line ? "Line" : "Polyline");
    query.addBindValue(serializePoints(item.points()));
    query.addBindValue(item.lengthInches());
    query.addBindValue(item.qty());
    query.addBindValue(item.shapeId() > 0 ? item.shapeId() : QVariant());
    query.addBindValue(item.designation());
    query.addBindValue(item.notes());
    query.addBindValue(item.id());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool ProjectDatabase::deleteTakeoffItem(int itemId)
{
    if (!m_isOpen) return false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM takeoff_items WHERE id = ?");
    query.addBindValue(itemId);
    
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

TakeoffItem ProjectDatabase::getTakeoffItem(int itemId) const
{
    TakeoffItem item;
    if (!m_isOpen) return item;

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM takeoff_items WHERE id = ?");
    query.addBindValue(itemId);
    
    if (query.exec() && query.next()) {
        item.setId(query.value("id").toInt());
        item.setPageId(query.value("page_id").toString());
        item.setKind(query.value("kind").toString() == "Line" ? TakeoffItem::Line : TakeoffItem::Polyline);
        item.setPoints(deserializePoints(query.value("points").toString()));
        item.setLengthInches(query.value("length_in").toDouble());
        item.setQty(query.value("qty").toInt());
        item.setShapeId(query.value("shape_id").toInt());
        item.setDesignation(query.value("designation").toString());
        item.setNotes(query.value("notes").toString());
    }
    return item;
}

QVector<TakeoffItem> ProjectDatabase::getTakeoffItemsForPage(const QString& pageId) const
{
    QVector<TakeoffItem> items;
    if (!m_isOpen) return items;

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM takeoff_items WHERE page_id = ? ORDER BY id");
    query.addBindValue(pageId);
    
    if (query.exec()) {
        while (query.next()) {
            TakeoffItem item;
            item.setId(query.value("id").toInt());
            item.setPageId(query.value("page_id").toString());
            item.setKind(query.value("kind").toString() == "Line" ? TakeoffItem::Line : TakeoffItem::Polyline);
            item.setPoints(deserializePoints(query.value("points").toString()));
            item.setLengthInches(query.value("length_in").toDouble());
            item.setQty(query.value("qty").toInt());
            item.setShapeId(query.value("shape_id").toInt());
            item.setDesignation(query.value("designation").toString());
            item.setNotes(query.value("notes").toString());
            items.append(item);
        }
    }
    return items;
}

QVector<TakeoffItem> ProjectDatabase::getAllTakeoffItems() const
{
    QVector<TakeoffItem> items;
    if (!m_isOpen) return items;

    QSqlQuery query(m_db);
    query.exec("SELECT * FROM takeoff_items ORDER BY id");
    
    while (query.next()) {
        TakeoffItem item;
        item.setId(query.value("id").toInt());
        item.setPageId(query.value("page_id").toString());
        item.setKind(query.value("kind").toString() == "Line" ? TakeoffItem::Line : TakeoffItem::Polyline);
        item.setPoints(deserializePoints(query.value("points").toString()));
        item.setLengthInches(query.value("length_in").toDouble());
        item.setQty(query.value("qty").toInt());
        item.setShapeId(query.value("shape_id").toInt());
        item.setDesignation(query.value("designation").toString());
        item.setNotes(query.value("notes").toString());
        items.append(item);
    }
    return items;
}

// =========================================================================
// Shapes
// =========================================================================

int ProjectDatabase::insertShape(const QString& designation, const QString& shapeType, double wLbPerFt)
{
    if (!m_isOpen) return -1;

    QSqlQuery query(m_db);
    query.prepare("INSERT OR REPLACE INTO shapes (designation, shape_type, w_lb_per_ft) VALUES (?, ?, ?)");
    query.addBindValue(designation);
    query.addBindValue(shapeType);
    query.addBindValue(wLbPerFt);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

bool ProjectDatabase::updateShape(int shapeId, const QString& designation, const QString& shapeType, double wLbPerFt)
{
    if (!m_isOpen) return false;

    QSqlQuery query(m_db);
    query.prepare("UPDATE shapes SET designation = ?, shape_type = ?, w_lb_per_ft = ? WHERE id = ?");
    query.addBindValue(designation);
    query.addBindValue(shapeType);
    query.addBindValue(wLbPerFt);
    query.addBindValue(shapeId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool ProjectDatabase::deleteShape(int shapeId)
{
    if (!m_isOpen) return false;

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM shapes WHERE id = ?");
    query.addBindValue(shapeId);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

ProjectDatabase::Shape ProjectDatabase::getShape(int shapeId) const
{
    Shape shape;
    if (!m_isOpen || shapeId <= 0) return shape;

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM shapes WHERE id = ?");
    query.addBindValue(shapeId);
    
    if (query.exec() && query.next()) {
        shape.id = query.value("id").toInt();
        shape.designation = query.value("designation").toString();
        shape.shapeType = query.value("shape_type").toString();
        shape.wLbPerFt = query.value("w_lb_per_ft").toDouble();
    }
    return shape;
}

ProjectDatabase::Shape ProjectDatabase::getShapeByDesignation(const QString& designation) const
{
    Shape shape;
    if (!m_isOpen || designation.isEmpty()) return shape;

    QSqlQuery query(m_db);
    query.prepare("SELECT * FROM shapes WHERE designation = ?");
    query.addBindValue(designation);
    
    if (query.exec() && query.next()) {
        shape.id = query.value("id").toInt();
        shape.designation = query.value("designation").toString();
        shape.shapeType = query.value("shape_type").toString();
        shape.wLbPerFt = query.value("w_lb_per_ft").toDouble();
    }
    return shape;
}

QVector<ProjectDatabase::Shape> ProjectDatabase::getAllShapes() const
{
    QVector<Shape> shapes;
    if (!m_isOpen) return shapes;

    QSqlQuery query(m_db);
    query.exec("SELECT * FROM shapes ORDER BY designation");
    
    while (query.next()) {
        Shape shape;
        shape.id = query.value("id").toInt();
        shape.designation = query.value("designation").toString();
        shape.shapeType = query.value("shape_type").toString();
        shape.wLbPerFt = query.value("w_lb_per_ft").toDouble();
        shapes.append(shape);
    }
    return shapes;
}

QVector<ProjectDatabase::Shape> ProjectDatabase::searchShapes(const QString& searchText, const QString& typeFilter, int limit) const
{
    QVector<Shape> shapes;
    if (!m_isOpen) return shapes;

    QString sql = "SELECT * FROM shapes WHERE 1=1";
    QVector<QString> params;

    if (!searchText.isEmpty()) {
        sql += " AND designation LIKE ?";
        params.append("%" + searchText + "%");
    }
    if (!typeFilter.isEmpty()) {
        sql += " AND shape_type = ?";
        params.append(typeFilter);
    }
    sql += " ORDER BY designation LIMIT ?";

    QSqlQuery query(m_db);
    query.prepare(sql);
    for (const QString& p : params) {
        query.addBindValue(p);
    }
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            Shape shape;
            shape.id = query.value("id").toInt();
            shape.designation = query.value("designation").toString();
            shape.shapeType = query.value("shape_type").toString();
            shape.wLbPerFt = query.value("w_lb_per_ft").toDouble();
            shapes.append(shape);
        }
    }
    return shapes;
}

QStringList ProjectDatabase::getAllDesignations() const
{
    QStringList designations;
    if (!m_isOpen) return designations;

    QSqlQuery query(m_db);
    query.exec("SELECT designation FROM shapes ORDER BY designation");
    
    while (query.next()) {
        designations.append(query.value(0).toString());
    }
    return designations;
}

QStringList ProjectDatabase::getShapeTypes() const
{
    QStringList types;
    if (!m_isOpen) return types;

    QSqlQuery query(m_db);
    query.exec("SELECT DISTINCT shape_type FROM shapes ORDER BY shape_type");
    
    while (query.next()) {
        types.append(query.value(0).toString());
    }
    return types;
}

int ProjectDatabase::getShapeCount() const
{
    if (!m_isOpen) return 0;

    QSqlQuery query(m_db);
    query.exec("SELECT COUNT(*) FROM shapes");
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

bool ProjectDatabase::hasShapes() const
{
    return getShapeCount() > 0;
}

void ProjectDatabase::clearShapes()
{
    if (!m_isOpen) return;
    QSqlQuery query(m_db);
    query.exec("DELETE FROM shapes");
}

int ProjectDatabase::importShapesFromCsv(const QString& filePath)
{
    if (!m_isOpen) return -1;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = "Could not open file: " + filePath;
        return -1;
    }

    QTextStream in(&file);
    QString headerLine = in.readLine();
    if (headerLine.isEmpty()) {
        m_lastError = "Empty CSV file";
        return -1;
    }

    // Parse header to find column indices
    QStringList headers = headerLine.split(',');
    int desigCol = -1, typeCol = -1, weightCol = -1;

    for (int i = 0; i < headers.size(); ++i) {
        QString h = headers[i].trimmed().toUpper();
        if (h.contains("AISC") && h.contains("LABEL")) desigCol = i;
        else if (h == "TYPE" || h == "SHAPE_TYPE") typeCol = i;
        else if (h == "W" || h == "W(LB/FT)" || h.contains("WEIGHT") || h.contains("LB/FT")) weightCol = i;
        // Fallback: first column is often the designation
        if (desigCol == -1 && i == 0) desigCol = i;
    }

    if (desigCol == -1) {
        m_lastError = "Could not find designation column";
        return -1;
    }

    // Start transaction for performance
    m_db.transaction();

    int imported = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList fields = line.split(',');
        if (fields.size() <= desigCol) continue;

        QString designation = fields[desigCol].trimmed();
        if (designation.isEmpty()) continue;

        QString shapeType;
        if (typeCol >= 0 && typeCol < fields.size()) {
            shapeType = fields[typeCol].trimmed();
        } else {
            // Derive type from designation prefix
            if (designation.startsWith("W")) shapeType = "W";
            else if (designation.startsWith("HSS")) shapeType = "HSS";
            else if (designation.startsWith("C")) shapeType = "C";
            else if (designation.startsWith("L")) shapeType = "L";
            else if (designation.startsWith("WT")) shapeType = "WT";
            else if (designation.startsWith("MC")) shapeType = "MC";
            else if (designation.startsWith("PIPE")) shapeType = "PIPE";
            else if (designation.startsWith("ST")) shapeType = "ST";
            else if (designation.startsWith("HP")) shapeType = "HP";
            else shapeType = "OTHER";
        }

        double weight = 0.0;
        if (weightCol >= 0 && weightCol < fields.size()) {
            weight = fields[weightCol].trimmed().toDouble();
        }

        if (insertShape(designation, shapeType, weight) > 0) {
            imported++;
        }
    }

    m_db.commit();
    file.close();

    return imported;
}

QString ProjectDatabase::lastError() const
{
    return m_lastError;
}

