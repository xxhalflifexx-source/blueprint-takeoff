#ifndef QUOTEDOCK_H
#define QUOTEDOCK_H

#include <QDockWidget>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

#include "../models/Project.h"
#include "../models/TakeoffItem.h"

/**
 * @brief Dock widget for quote summary display with weight/cost calculations.
 * 
 * Shows a table grouped by designation with columns:
 * - Designation
 * - Qty (total items)
 * - Total Length (ft)
 * - W (lb/ft)
 * - Total Weight (lb)
 * - $/lb
 * - Material Cost ($)
 * 
 * Footer shows grand totals and editable $/lb rate.
 */
class QuoteDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit QuoteDock(QWidget* parent = nullptr);
    ~QuoteDock();

    /**
     * @brief Update the quote summary from project data.
     * @param project The project to calculate from
     */
    void updateFromProject(Project* project);

    /**
     * @brief Get the current material price per lb.
     */
    double materialPricePerLb() const;

    /**
     * @brief Set the material price per lb.
     */
    void setMaterialPricePerLb(double pricePerLb);

    /**
     * @brief Check if "Current Page Only" filter is enabled.
     */
    bool isCurrentPageOnly() const;

signals:
    /**
     * @brief Emitted when material price per lb is changed by user.
     */
    void materialPriceChanged(double pricePerLb);

    /**
     * @brief Emitted when "Current Page Only" checkbox changes.
     */
    void currentPageOnlyChanged(bool currentPageOnly);

private slots:
    void onExportCsv();
    void onPriceChanged();
    void onCurrentPageOnlyToggled(bool checked);

private:
    void setupUi();
    void populateTable(Project* project, const QString& pageFilter = QString());
    void updateTotals(double totalWeight, double totalCost, int totalQty);

    // Container
    QWidget* m_container;

    // Table
    QTableWidget* m_table;

    // Filter checkbox
    QCheckBox* m_currentPageOnlyCheck;

    // Price input
    QDoubleSpinBox* m_pricePerLbSpin;

    // Totals labels
    QLabel* m_totalWeightLabel;
    QLabel* m_totalCostLabel;
    QLabel* m_totalQtyLabel;

    // Export button
    QPushButton* m_exportButton;

    // Cached project pointer for recalculation
    Project* m_cachedProject;
    QString m_currentPageId;
};

#endif // QUOTEDOCK_H
