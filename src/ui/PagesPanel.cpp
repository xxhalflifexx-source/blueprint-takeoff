#include "PagesPanel.h"
#include "../models/Page.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QAction>

PagesPanel::PagesPanel(QWidget* parent)
    : QWidget(parent)
    , m_listWidget(new QListWidget(this))
    , m_deleteButton(new QPushButton("Delete Page", this))
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    // Header label
    QLabel* header = new QLabel("Pages", this);
    header->setStyleSheet("font-weight: bold; padding: 4px;");
    layout->addWidget(header);

    // Delete button
    m_deleteButton->setEnabled(false);
    m_deleteButton->setToolTip("Delete the selected page");
    layout->addWidget(m_deleteButton);

    // List widget
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_listWidget);

    // Connections
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, &PagesPanel::onItemSelectionChanged);
    connect(m_listWidget, &QListWidget::customContextMenuRequested,
            this, &PagesPanel::onContextMenu);
    connect(m_deleteButton, &QPushButton::clicked,
            this, &PagesPanel::onDeleteButtonClicked);
}

void PagesPanel::addPage(const Page& page)
{
    QListWidgetItem* item = new QListWidgetItem(page.listDisplayString());
    item->setToolTip(page.sourcePath());
    
    m_listWidget->addItem(item);
    m_idToItem[page.id()] = item;
    m_itemToId[item] = page.id();

    // Select the first page automatically
    if (m_listWidget->count() == 1) {
        m_listWidget->setCurrentItem(item);
    }
}

void PagesPanel::removePage(const QString& pageId)
{
    if (!m_idToItem.contains(pageId)) {
        return;
    }

    QListWidgetItem* item = m_idToItem[pageId];
    int row = m_listWidget->row(item);
    
    m_idToItem.remove(pageId);
    m_itemToId.remove(item);
    
    if (row >= 0) {
        delete m_listWidget->takeItem(row);
    }
}

void PagesPanel::clearPages()
{
    m_listWidget->clear();
    m_idToItem.clear();
    m_itemToId.clear();
    m_deleteButton->setEnabled(false);
}

void PagesPanel::updatePage(const Page& page)
{
    if (!m_idToItem.contains(page.id())) {
        return;
    }

    QListWidgetItem* item = m_idToItem[page.id()];
    item->setText(page.listDisplayString());
    item->setToolTip(page.sourcePath());
}

void PagesPanel::selectPage(const QString& pageId)
{
    if (!m_idToItem.contains(pageId)) {
        return;
    }

    QListWidgetItem* item = m_idToItem[pageId];
    m_listWidget->setCurrentItem(item);
}

QString PagesPanel::selectedPageId() const
{
    QListWidgetItem* current = m_listWidget->currentItem();
    if (current && m_itemToId.contains(current)) {
        return m_itemToId[current];
    }
    return QString();
}

int PagesPanel::pageCount() const
{
    return m_listWidget->count();
}

void PagesPanel::onItemSelectionChanged()
{
    QString pageId = selectedPageId();
    m_deleteButton->setEnabled(!pageId.isEmpty());
    if (!pageId.isEmpty()) {
        emit pageSelected(pageId);
    }
}

void PagesPanel::setDeleteButtonEnabled(bool enabled)
{
    m_deleteButton->setEnabled(enabled);
}

void PagesPanel::onDeleteButtonClicked()
{
    QString pageId = selectedPageId();
    if (!pageId.isEmpty()) {
        emit pageDeleteRequested(pageId);
    }
}

void PagesPanel::onContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = m_listWidget->itemAt(pos);
    if (!item || !m_itemToId.contains(item)) {
        return;
    }

    QString pageId = m_itemToId[item];

    QMenu menu(this);
    QAction* deleteAction = menu.addAction("Delete Page");
    
    QAction* selected = menu.exec(m_listWidget->mapToGlobal(pos));
    if (selected == deleteAction) {
        emit pageDeleteRequested(pageId);
    }
}

