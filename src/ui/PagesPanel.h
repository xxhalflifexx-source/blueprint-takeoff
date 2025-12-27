#ifndef PAGESPANEL_H
#define PAGESPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QMap>

class Page;

/**
 * @brief Panel widget showing the list of pages in the project.
 * 
 * Displays pages with their type (IMG/PDF) and allows selection
 * to switch the active page in the viewer.
 */
class PagesPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PagesPanel(QWidget* parent = nullptr);
    ~PagesPanel() override = default;

    /**
     * @brief Add a page to the list.
     * @param page Page to add
     */
    void addPage(const Page& page);

    /**
     * @brief Remove a page from the list.
     * @param pageId Page ID to remove
     */
    void removePage(const QString& pageId);

    /**
     * @brief Clear all pages from the list.
     */
    void clearPages();

    /**
     * @brief Update the display of a page.
     * @param page Page with updated info
     */
    void updatePage(const Page& page);

    /**
     * @brief Select a page by ID.
     * @param pageId Page ID to select
     */
    void selectPage(const QString& pageId);

    /**
     * @brief Get the currently selected page ID.
     * @return Page ID, or empty if none selected
     */
    QString selectedPageId() const;

    /**
     * @brief Get the number of pages.
     * @return Page count
     */
    int pageCount() const;

    /**
     * @brief Enable or disable the delete button.
     * @param enabled true to enable, false to disable
     */
    void setDeleteButtonEnabled(bool enabled);

signals:
    /**
     * @brief Emitted when a page is selected.
     * @param pageId ID of the selected page
     */
    void pageSelected(const QString& pageId);

    /**
     * @brief Emitted when user requests to delete a page.
     * @param pageId ID of the page to delete
     */
    void pageDeleteRequested(const QString& pageId);

private slots:
    void onItemSelectionChanged();
    void onContextMenu(const QPoint& pos);
    void onDeleteButtonClicked();

private:
    QListWidget* m_listWidget;
    QPushButton* m_deleteButton;
    QMap<QString, QListWidgetItem*> m_idToItem;
    QMap<QListWidgetItem*, QString> m_itemToId;
};

#endif // PAGESPANEL_H

