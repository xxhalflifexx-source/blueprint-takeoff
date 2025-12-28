#ifndef QUOTEDOCK_H
#define QUOTEDOCK_H

#include <QDockWidget>
#include <QTableWidget>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>

#include "Project.h"
#include "QuoteCalculator.h"
#include "ShapesDatabase.h"

/**
 * @brief Dock widget for quote summary display and export.
 * 
 * Shows a table grouped by Material Type + Size + Labor Class.
 * Includes rate inputs and totals with CSV export functionality.
 * Supports filtering to current page only.
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
    void updateFromProject(const Project& project);

    /**
     * @brief Update the quote summary from measurements.
     * @param measurements The measurements to calculate from
     * @param rates The quote rates to use
     * @param shapesDb Optional shapes database for weight lookup
     */
    void updateFromMeasurements(const QVector<Measurement>& measurements, 
                                 const QuoteRates& rates,
                                 ShapesDatabase* shapesDb = nullptr);

    /**
     * @brief Get the current quote rates from UI.
     * @return QuoteRates struct
     */
    QuoteRates currentRates() const;

    /**
     * @brief Set the quote rates in UI.
     * @param rates Rates to display
     */
    void setRates(const QuoteRates& rates);

    /**
     * @brief Check if "Current Page Only" filter is enabled.
     * @return true if filtering to current page
     */
    bool isCurrentPageOnly() const;

signals:
    /**
     * @brief Emitted when rate values are changed by user.
     */
    void ratesChanged(const QuoteRates& rates);

    /**
     * @brief Emitted when "Current Page Only" checkbox changes.
     */
    void currentPageOnlyChanged(bool currentPageOnly);

private slots:
    void onExportCsv();
    void onRateChanged();
    void onCurrentPageOnlyToggled(bool checked);

private:
    void setupUi();
    void populateTable(const QuoteSummary& summary);
    void updateTotals(const QuoteSummary& summary);

    // Container
    QWidget* m_container;

    // Table
    QTableWidget* m_table;

    // Filter checkbox
    QCheckBox* m_currentPageOnlyCheck;

    // Rate inputs
    QDoubleSpinBox* m_materialRateSpin;
    QDoubleSpinBox* m_laborRateSpin;
    QDoubleSpinBox* m_markupSpin;

    // Totals labels
    QLabel* m_subtotalLabel;
    QLabel* m_totalLabel;
    QLabel* m_weightLabel;

    // Export button
    QPushButton* m_exportButton;

    // Calculator
    QuoteCalculator m_calculator;

    // Cached project data for recalculation
    QVector<Measurement> m_cachedMeasurements;
    ShapesDatabase* m_shapesDb;
};

#endif // QUOTEDOCK_H
