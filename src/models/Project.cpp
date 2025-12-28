#include "Project.h"

const QString Project::FILE_EXTENSION = ".takeoff.db";
const QString Project::FILE_FILTER = "Takeoff Project (*.takeoff.db);;All Files (*)";

Project::Project()
    : m_db(std::make_unique<ProjectDatabase>())
{
}

Project::~Project()
{
    close();
}

// ============================================================================
// Project File Operations
// ============================================================================

bool Project::create(const QString& filePath)
{
    close();

    if (!m_db->create(filePath)) {
        m_lastError = m_db->lastError();
        return false;
    }

    m_pages.clear();
    m_takeoffItems.clear();
    return true;
}

bool Project::open(const QString& filePath)
{
    close();

    if (!m_db->open(filePath)) {
        m_lastError = m_db->lastError();
        return false;
    }

    reloadPages();
    reloadTakeoffItems();
    return true;
}

void Project::close()
{
    if (m_db->isOpen()) {
        m_db->close();
    }
    m_pages.clear();
    m_takeoffItems.clear();
}

bool Project::isOpen() const
{
    return m_db->isOpen();
}

QString Project::filePath() const
{
    return m_db->filePath();
}

// ============================================================================
// Project Settings
// ============================================================================

QString Project::name() const
{
    return m_db->getProjectName();
}

void Project::setName(const QString& name)
{
    m_db->setProjectName(name);
}

double Project::materialPricePerLb() const
{
    return m_db->getMaterialPricePerLb();
}

void Project::setMaterialPricePerLb(double pricePerLb)
{
    m_db->setMaterialPricePerLb(pricePerLb);
}

// ============================================================================
// Pages
// ============================================================================

const QVector<Page>& Project::pages() const
{
    return m_pages;
}

void Project::addPage(const Page& page)
{
    if (m_db->insertPage(page)) {
        m_pages.append(page);
    } else {
        m_lastError = m_db->lastError();
    }
}

void Project::removePage(const QString& pageId)
{
    if (m_db->deletePage(pageId)) {
        // Remove from in-memory cache
        for (int i = m_pages.size() - 1; i >= 0; --i) {
            if (m_pages[i].id() == pageId) {
                m_pages.removeAt(i);
                break;
            }
        }
        // Remove associated items from cache
        for (int i = m_takeoffItems.size() - 1; i >= 0; --i) {
            if (m_takeoffItems[i].pageId() == pageId) {
                m_takeoffItems.removeAt(i);
            }
        }
    } else {
        m_lastError = m_db->lastError();
    }
}

void Project::updatePage(const Page& page)
{
    if (m_db->updatePage(page)) {
        // Update in-memory cache
        for (int i = 0; i < m_pages.size(); ++i) {
            if (m_pages[i].id() == page.id()) {
                m_pages[i] = page;
                break;
            }
        }
    } else {
        m_lastError = m_db->lastError();
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

void Project::reloadPages()
{
    m_pages = m_db->getAllPages();
}

// ============================================================================
// Takeoff Items
// ============================================================================

const QVector<TakeoffItem>& Project::takeoffItems() const
{
    return m_takeoffItems;
}

QVector<TakeoffItem> Project::takeoffItemsForPage(const QString& pageId) const
{
    QVector<TakeoffItem> result;
    for (const TakeoffItem& item : m_takeoffItems) {
        if (item.pageId() == pageId) {
            result.append(item);
        }
    }
    return result;
}

int Project::addTakeoffItem(TakeoffItem& item)
{
    int newId = m_db->insertTakeoffItem(item);
    if (newId > 0) {
        item.setId(newId);
        m_takeoffItems.append(item);
        return newId;
    }
    m_lastError = m_db->lastError();
    return -1;
}

void Project::updateTakeoffItem(const TakeoffItem& item)
{
    if (m_db->updateTakeoffItem(item)) {
        // Update in-memory cache
        for (int i = 0; i < m_takeoffItems.size(); ++i) {
            if (m_takeoffItems[i].id() == item.id()) {
                m_takeoffItems[i] = item;
                break;
            }
        }
    } else {
        m_lastError = m_db->lastError();
    }
}

void Project::removeTakeoffItem(int id)
{
    if (m_db->deleteTakeoffItem(id)) {
        for (int i = m_takeoffItems.size() - 1; i >= 0; --i) {
            if (m_takeoffItems[i].id() == id) {
                m_takeoffItems.removeAt(i);
                break;
            }
        }
    } else {
        m_lastError = m_db->lastError();
    }
}

TakeoffItem* Project::findTakeoffItem(int id)
{
    for (int i = 0; i < m_takeoffItems.size(); ++i) {
        if (m_takeoffItems[i].id() == id) {
            return &m_takeoffItems[i];
        }
    }
    return nullptr;
}

const TakeoffItem* Project::findTakeoffItem(int id) const
{
    for (int i = 0; i < m_takeoffItems.size(); ++i) {
        if (m_takeoffItems[i].id() == id) {
            return &m_takeoffItems[i];
        }
    }
    return nullptr;
}

void Project::reloadTakeoffItems()
{
    m_takeoffItems = m_db->getAllTakeoffItems();
}

// ============================================================================
// Shapes
// ============================================================================

bool Project::hasShapes() const
{
    return m_db->hasShapes();
}

int Project::shapeCount() const
{
    return m_db->getShapeCount();
}

QStringList Project::allDesignations() const
{
    return m_db->getAllDesignations();
}

QStringList Project::shapeTypes() const
{
    return m_db->getShapeTypes();
}

QVector<ProjectDatabase::Shape> Project::searchShapes(const QString& text,
                                                       const QString& typeFilter,
                                                       int limit) const
{
    return m_db->searchShapes(text, typeFilter, limit);
}

ProjectDatabase::Shape Project::getShape(int shapeId) const
{
    return m_db->getShape(shapeId);
}

ProjectDatabase::Shape Project::getShapeByDesignation(const QString& designation) const
{
    return m_db->getShapeByDesignation(designation);
}

int Project::importShapesFromCsv(const QString& csvPath)
{
    return m_db->importShapesFromCsv(csvPath);
}

// ============================================================================
// Error Handling
// ============================================================================

QString Project::lastError() const
{
    return m_lastError;
}
