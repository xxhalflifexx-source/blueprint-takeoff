#ifndef MEASUREMENTPANEL_H
#define MEASUREMENTPANEL_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QMap>

#include "Measurement.h"

/**
 * @brief Panel displaying the list of completed measurements.
 * 
 * Shows all measurements with their type and length.
 * Allows selection to highlight on the blueprint.
 */
class MeasurementPanel : public QWidget
{
    Q_OBJECT

public:
    explicit MeasurementPanel(QWidget* parent = nullptr);
    ~MeasurementPanel();

    /**
     * @brief Add a measurement to the list.
     * @param measurement The measurement to add
     */
    void addMeasurement(const Measurement& measurement);

    /**
     * @brief Update a measurement's display in the list.
     * @param measurement The measurement with updated data
     */
    void updateMeasurement(const Measurement& measurement);

    /**
     * @brief Remove a measurement from the list.
     * @param measurementId The ID of the measurement to remove
     */
    void removeMeasurement(int measurementId);

    /**
     * @brief Clear all measurements from the list.
     */
    void clearMeasurements();

    /**
     * @brief Get the currently selected measurement ID.
     * @return Selected measurement ID, or -1 if none
     */
    int selectedMeasurementId() const;

signals:
    /**
     * @brief Emitted when a measurement is selected in the list.
     * @param measurementId The ID of the selected measurement
     */
    void measurementSelected(int measurementId);

private slots:
    void onSelectionChanged();

private:
    void setupUi();

    QVBoxLayout* m_layout;
    QLabel* m_titleLabel;
    QListWidget* m_listWidget;
    
    // Map from list item to measurement ID
    QMap<QListWidgetItem*, int> m_itemToId;
    // Reverse map for lookup by ID
    QMap<int, QListWidgetItem*> m_idToItem;
};

#endif // MEASUREMENTPANEL_H
