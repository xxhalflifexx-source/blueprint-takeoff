#include "ShapesDatabase.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

#ifdef HAS_QXLSX
#include "xlsxdocument.h"
#endif

ShapesDatabase::ShapesDatabase()
    : m_connectionName("ShapesDB")
    , m_isOpen(false)
{
}

ShapesDatabase::~ShapesDatabase()
{
    close();
}

bool ShapesDatabase::open()
{
    if (m_isOpen) {
        return true;
    }

    // Get app data location
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString dbPath = dir.filePath("shapes.db");

    // Create database connection
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        m_lastError = QString("Failed to open database: %1").arg(m_db.lastError().text());
        return false;
    }

    createSchema();
    m_isOpen = true;
    return true;
}

void ShapesDatabase::close()
{
    if (m_isOpen) {
        m_db.close();
        m_isOpen = false;
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool ShapesDatabase::isOpen() const
{
    return m_isOpen;
}

void ShapesDatabase::createSchema()
{
    QSqlQuery query(m_db);

    // Shapes table
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS shapes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            shape_key TEXT UNIQUE,
            aisc_label TEXT,
            edi_name TEXT,
            shape_type TEXT
        )
    )");

    // Shape properties table (EAV pattern for flexibility)
    query.exec(R"(
        CREATE TABLE IF NOT EXISTS shape_props (
            shape_id INTEGER,
            prop_key TEXT,
            prop_value TEXT,
            prop_value_num REAL,
            PRIMARY KEY(shape_id, prop_key),
            FOREIGN KEY(shape_id) REFERENCES shapes(id) ON DELETE CASCADE
        )
    )");

    // Indexes for common queries
    query.exec("CREATE INDEX IF NOT EXISTS idx_shapes_type ON shapes(shape_type)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_shapes_label ON shapes(aisc_label)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_props_key ON shape_props(prop_key)");
}

int ShapesDatabase::importFromXlsx(const QString& filePath)
{
#ifdef HAS_QXLSX
    if (!m_isOpen) {
        m_lastError = "Database not open";
        return -1;
    }

    QXlsx::Document xlsx(filePath);
    if (!xlsx.load()) {
        m_lastError = QString("Failed to load XLSX file: %1").arg(filePath);
        return -1;
    }

    // Get the first sheet
    QStringList sheetNames = xlsx.sheetNames();
    if (sheetNames.isEmpty()) {
        m_lastError = "XLSX file has no sheets";
        return -1;
    }

    xlsx.selectSheet(sheetNames.first());

    // Find dimensions
    QXlsx::CellRange range = xlsx.dimension();
    if (!range.isValid()) {
        m_lastError = "Cannot determine spreadsheet dimensions";
        return -1;
    }

    int firstRow = range.firstRow();
    int lastRow = range.lastRow();
    int firstCol = range.firstColumn();
    int lastCol = range.lastColumn();

    // Find header row (first row with multiple non-empty text cells)
    int headerRow = -1;
    QStringList headers;
    
    for (int row = firstRow; row <= qMin(firstRow + 10, lastRow); ++row) {
        QStringList rowHeaders;
        int nonEmptyCount = 0;
        
        for (int col = firstCol; col <= lastCol; ++col) {
            QXlsx::Cell* cell = xlsx.cellAt(row, col);
            QString value = cell ? cell->value().toString().trimmed() : QString();
            rowHeaders.append(value);
            if (!value.isEmpty()) {
                nonEmptyCount++;
            }
        }
        
        // Header row typically has many non-empty string cells
        if (nonEmptyCount >= 5) {
            headerRow = row;
            headers = rowHeaders;
            break;
        }
    }

    if (headerRow < 0) {
        m_lastError = "Could not find header row in spreadsheet";
        return -1;
    }

    // Start transaction for performance
    m_db.transaction();

    QSqlQuery insertShape(m_db);
    insertShape.prepare(R"(
        INSERT OR REPLACE INTO shapes (shape_key, aisc_label, edi_name, shape_type)
        VALUES (:key, :label, :edi, :type)
    )");

    QSqlQuery insertProp(m_db);
    insertProp.prepare(R"(
        INSERT OR REPLACE INTO shape_props (shape_id, prop_key, prop_value, prop_value_num)
        VALUES (:shape_id, :key, :value, :num)
    )");

    // Find important column indices
    int labelColIdx = -1;
    int ediColIdx = -1;
    
    QStringList labelNames = {"AISC_Manual_Label", "AISC Manual Label", "Label", "Shape"};
    QStringList ediNames = {"EDI_Std_Nomenclature", "EDI Name", "EDI", "Nomenclature"};
    
    for (int i = 0; i < headers.size(); ++i) {
        QString h = headers[i];
        for (const QString& name : labelNames) {
            if (h.compare(name, Qt::CaseInsensitive) == 0) {
                labelColIdx = i;
                break;
            }
        }
        for (const QString& name : ediNames) {
            if (h.compare(name, Qt::CaseInsensitive) == 0) {
                ediColIdx = i;
                break;
            }
        }
    }

    int importCount = 0;

    // Import data rows
    for (int row = headerRow + 1; row <= lastRow; ++row) {
        QVector<QString> rowData;
        
        for (int col = firstCol; col <= lastCol; ++col) {
            QXlsx::Cell* cell = xlsx.cellAt(row, col);
            QString value = cell ? cell->value().toString().trimmed() : QString();
            rowData.append(value);
        }

        // Determine label and EDI name
        QString aiscLabel;
        QString ediName;
        
        if (labelColIdx >= 0 && labelColIdx < rowData.size()) {
            aiscLabel = rowData[labelColIdx];
        }
        if (ediColIdx >= 0 && ediColIdx < rowData.size()) {
            ediName = rowData[ediColIdx];
        }

        // Skip rows without a label
        if (aiscLabel.isEmpty() && ediName.isEmpty()) {
            continue;
        }

        // Use EDI name as shape key if available, else label
        QString shapeKey = ediName.isEmpty() ? aiscLabel : ediName;
        if (shapeKey.isEmpty()) {
            continue;
        }

        // Determine shape type from label prefix
        QString shapeType = determineShapeType(aiscLabel.isEmpty() ? ediName : aiscLabel);

        // Insert shape
        insertShape.bindValue(":key", shapeKey);
        insertShape.bindValue(":label", aiscLabel);
        insertShape.bindValue(":edi", ediName);
        insertShape.bindValue(":type", shapeType);
        
        if (!insertShape.exec()) {
            continue;  // Skip duplicates
        }

        // Get inserted shape ID
        int shapeId = insertShape.lastInsertId().toInt();

        // Insert all properties
        for (int i = 0; i < headers.size() && i < rowData.size(); ++i) {
            QString propKey = headers[i];
            QString propValue = rowData[i];
            
            if (propKey.isEmpty() || propValue.isEmpty()) {
                continue;
            }

            // Try to parse as number
            bool ok;
            double numValue = propValue.toDouble(&ok);

            insertProp.bindValue(":shape_id", shapeId);
            insertProp.bindValue(":key", propKey);
            insertProp.bindValue(":value", propValue);
            insertProp.bindValue(":num", ok ? numValue : QVariant());
            insertProp.exec();
        }

        importCount++;
    }

    m_db.commit();
    return importCount;
#else
    m_lastError = "XLSX import not available. Please use CSV format instead.";
    return -1;
#endif
}

int ShapesDatabase::importFromCsv(const QString& filePath)
{
    if (!m_isOpen) {
        m_lastError = "Database not open";
        return -1;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_lastError = QString("Failed to open CSV file: %1").arg(filePath);
        return -1;
    }

    QTextStream in(&file);
    
    // Read header line
    if (in.atEnd()) {
        m_lastError = "CSV file is empty";
        return -1;
    }

    QString headerLine = in.readLine();
    QStringList headers = headerLine.split(',');
    
    // Clean up headers
    for (int i = 0; i < headers.size(); ++i) {
        headers[i] = headers[i].trimmed().remove('"');
    }

    // Find important column indices
    int labelColIdx = -1;
    int ediColIdx = -1;
    
    QStringList labelNames = {"AISC_Manual_Label", "AISC Manual Label", "Label", "Shape"};
    QStringList ediNames = {"EDI_Std_Nomenclature", "EDI Name", "EDI", "Nomenclature"};
    
    for (int i = 0; i < headers.size(); ++i) {
        QString h = headers[i];
        for (const QString& name : labelNames) {
            if (h.compare(name, Qt::CaseInsensitive) == 0) {
                labelColIdx = i;
                break;
            }
        }
        for (const QString& name : ediNames) {
            if (h.compare(name, Qt::CaseInsensitive) == 0) {
                ediColIdx = i;
                break;
            }
        }
    }

    m_db.transaction();

    QSqlQuery insertShape(m_db);
    insertShape.prepare(R"(
        INSERT OR REPLACE INTO shapes (shape_key, aisc_label, edi_name, shape_type)
        VALUES (:key, :label, :edi, :type)
    )");

    QSqlQuery insertProp(m_db);
    insertProp.prepare(R"(
        INSERT OR REPLACE INTO shape_props (shape_id, prop_key, prop_value, prop_value_num)
        VALUES (:shape_id, :key, :value, :num)
    )");

    int importCount = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        
        // Simple CSV parsing (doesn't handle quoted commas perfectly)
        QStringList values = line.split(',');
        
        for (int i = 0; i < values.size(); ++i) {
            values[i] = values[i].trimmed().remove('"');
        }

        QString aiscLabel;
        QString ediName;
        
        if (labelColIdx >= 0 && labelColIdx < values.size()) {
            aiscLabel = values[labelColIdx];
        }
        if (ediColIdx >= 0 && ediColIdx < values.size()) {
            ediName = values[ediColIdx];
        }

        if (aiscLabel.isEmpty() && ediName.isEmpty()) {
            continue;
        }

        QString shapeKey = ediName.isEmpty() ? aiscLabel : ediName;
        if (shapeKey.isEmpty()) {
            continue;
        }

        QString shapeType = determineShapeType(aiscLabel.isEmpty() ? ediName : aiscLabel);

        insertShape.bindValue(":key", shapeKey);
        insertShape.bindValue(":label", aiscLabel);
        insertShape.bindValue(":edi", ediName);
        insertShape.bindValue(":type", shapeType);
        
        if (!insertShape.exec()) {
            continue;
        }

        int shapeId = insertShape.lastInsertId().toInt();

        for (int i = 0; i < headers.size() && i < values.size(); ++i) {
            QString propKey = headers[i];
            QString propValue = values[i];
            
            if (propKey.isEmpty() || propValue.isEmpty()) {
                continue;
            }

            bool ok;
            double numValue = propValue.toDouble(&ok);

            insertProp.bindValue(":shape_id", shapeId);
            insertProp.bindValue(":key", propKey);
            insertProp.bindValue(":value", propValue);
            insertProp.bindValue(":num", ok ? QVariant(numValue) : QVariant());
            insertProp.exec();
        }

        importCount++;
    }

    m_db.commit();
    return importCount;
}

QVector<ShapeRow> ShapesDatabase::queryShapes(const QString& typeFilter,
                                               const QString& searchText,
                                               int limit) const
{
    QVector<ShapeRow> results;

    if (!m_isOpen) {
        return results;
    }

    QString sql = R"(
        SELECT s.id, s.shape_key, s.aisc_label, s.edi_name, s.shape_type,
               COALESCE((SELECT prop_value_num FROM shape_props WHERE shape_id = s.id AND prop_key = 'W'), 0) as weight,
               COALESCE((SELECT prop_value_num FROM shape_props WHERE shape_id = s.id AND prop_key = 'd'), 0) as depth,
               COALESCE((SELECT prop_value_num FROM shape_props WHERE shape_id = s.id AND prop_key = 'bf'), 0) as flange
        FROM shapes s
        WHERE 1=1
    )";

    if (!typeFilter.isEmpty()) {
        sql += " AND s.shape_type = :type";
    }
    if (!searchText.isEmpty()) {
        sql += " AND (s.aisc_label LIKE :search OR s.edi_name LIKE :search OR s.shape_key LIKE :search)";
    }

    sql += " ORDER BY s.aisc_label LIMIT :limit";

    QSqlQuery query(m_db);
    query.prepare(sql);

    if (!typeFilter.isEmpty()) {
        query.bindValue(":type", typeFilter);
    }
    if (!searchText.isEmpty()) {
        query.bindValue(":search", "%" + searchText + "%");
    }
    query.bindValue(":limit", limit);

    if (query.exec()) {
        while (query.next()) {
            ShapeRow row;
            row.id = query.value(0).toInt();
            row.shapeKey = query.value(1).toString();
            row.aiscLabel = query.value(2).toString();
            row.ediName = query.value(3).toString();
            row.shapeType = query.value(4).toString();
            row.weightPerFt = query.value(5).toDouble();
            row.depth = query.value(6).toDouble();
            row.flangeWidth = query.value(7).toDouble();
            results.append(row);
        }
    }

    return results;
}

double ShapesDatabase::getShapeProperty(int shapeId, const QString& propKey) const
{
    if (!m_isOpen || shapeId < 0) {
        return 0.0;
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT prop_value_num FROM shape_props WHERE shape_id = :id AND prop_key = :key");
    query.bindValue(":id", shapeId);
    query.bindValue(":key", propKey);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

QString ShapesDatabase::getShapeLabel(int shapeId) const
{
    if (!m_isOpen || shapeId < 0) {
        return QString();
    }

    QSqlQuery query(m_db);
    query.prepare("SELECT aisc_label FROM shapes WHERE id = :id");
    query.bindValue(":id", shapeId);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }

    return QString();
}

QStringList ShapesDatabase::getShapeTypes() const
{
    QStringList types;

    if (!m_isOpen) {
        return types;
    }

    QSqlQuery query(m_db);
    query.exec("SELECT DISTINCT shape_type FROM shapes WHERE shape_type IS NOT NULL AND shape_type != '' ORDER BY shape_type");

    while (query.next()) {
        types.append(query.value(0).toString());
    }

    return types;
}

bool ShapesDatabase::hasShapes() const
{
    return shapeCount() > 0;
}

int ShapesDatabase::shapeCount() const
{
    if (!m_isOpen) {
        return 0;
    }

    QSqlQuery query(m_db);
    query.exec("SELECT COUNT(*) FROM shapes");

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

void ShapesDatabase::clearShapes()
{
    if (!m_isOpen) {
        return;
    }

    QSqlQuery query(m_db);
    query.exec("DELETE FROM shape_props");
    query.exec("DELETE FROM shapes");
}

QString ShapesDatabase::lastError() const
{
    return m_lastError;
}

QString ShapesDatabase::determineShapeType(const QString& label) const
{
    if (label.isEmpty()) {
        return "Other";
    }

    QString upper = label.toUpper();

    // Check for common prefixes
    static const QVector<QPair<QString, QString>> prefixes = {
        {"W", "W"},       // Wide flange
        {"M", "M"},       // Miscellaneous
        {"S", "S"},       // American Standard
        {"HP", "HP"},     // H-Pile
        {"C", "C"},       // Channel
        {"MC", "MC"},     // Miscellaneous Channel
        {"L", "L"},       // Angle
        {"WT", "WT"},     // Tee (cut from W)
        {"MT", "MT"},     // Tee (cut from M)
        {"ST", "ST"},     // Tee (cut from S)
        {"HSS", "HSS"},   // Hollow Structural Section
        {"PIPE", "PIPE"}, // Pipe
        {"2L", "2L"},     // Double Angle
    };

    for (const auto& pair : prefixes) {
        if (upper.startsWith(pair.first)) {
            // Make sure it's not a false match (e.g., "W" shouldn't match "WT")
            if (pair.first == "W" && (upper.startsWith("WT") || upper.startsWith("WP"))) {
                continue;
            }
            if (pair.first == "M" && (upper.startsWith("MC") || upper.startsWith("MT"))) {
                continue;
            }
            if (pair.first == "S" && upper.startsWith("ST")) {
                continue;
            }
            return pair.second;
        }
    }

    return "Other";
}

QString ShapesDatabase::findHeaderValue(const QStringList& headers, const QVector<QString>& row,
                                         const QStringList& possibleNames) const
{
    for (int i = 0; i < headers.size() && i < row.size(); ++i) {
        for (const QString& name : possibleNames) {
            if (headers[i].compare(name, Qt::CaseInsensitive) == 0) {
                return row[i];
            }
        }
    }
    return QString();
}

