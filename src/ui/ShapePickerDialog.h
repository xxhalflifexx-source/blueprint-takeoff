#ifndef SHAPEPICKERDIALOG_H
#define SHAPEPICKERDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>

#include "../core/ProjectDatabase.h"

/**
 * @brief Dialog for selecting an AISC shape from the database.
 * 
 * Provides filtering by shape type and search functionality.
 * Shows a table of matching shapes with properties.
 */
class ShapePickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShapePickerDialog(ProjectDatabase* db, QWidget* parent = nullptr);
    ~ShapePickerDialog();

    /**
     * @brief Get the selected shape ID.
     * @return Shape ID or -1 if none selected
     */
    int selectedShapeId() const;

    /**
     * @brief Get the selected shape label.
     * @return Designation or empty string if none selected
     */
    QString selectedShapeLabel() const;

    /**
     * @brief Get the selected shape's weight per foot.
     * @return Weight (lb/ft) or 0 if none selected
     */
    double selectedShapeWeight() const;

private slots:
    void onSearch();
    void onTypeFilterChanged(int index);
    void onTableSelectionChanged();
    void onTableDoubleClicked(int row, int column);

private:
    void setupUi();
    void populateTypeFilter();
    void refreshTable();

    ProjectDatabase* m_db;

    // UI Elements
    QComboBox* m_typeFilter;
    QLineEdit* m_searchBox;
    QTableWidget* m_table;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    QLabel* m_statusLabel;

    // Selection
    int m_selectedId;
    QString m_selectedLabel;
    double m_selectedWeight;
};

#endif // SHAPEPICKERDIALOG_H
