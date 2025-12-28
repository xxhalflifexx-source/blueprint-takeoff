#ifndef PROPERTIESDOCK_H
#define PROPERTIESDOCK_H

#include <QDockWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QCompleter>
#include <QStringListModel>

#include "../models/TakeoffItem.h"

/**
 * @brief Dock widget for editing takeoff item properties.
 * 
 * Simplified properties panel showing:
 * - Designation (with autocomplete)
 * - Qty (spinbox)
 * - Notes
 * - Computed: Length, Weight, Cost
 */
class PropertiesDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertiesDock(QWidget* parent = nullptr);
    ~PropertiesDock();

    /**
     * @brief Set the takeoff item to display/edit.
     * @param item Pointer to item (nullptr to clear)
     * @param id Item ID (for signal emission)
     */
    void setTakeoffItem(const TakeoffItem* item, int id);

    /**
     * @brief Clear the properties panel (no selection).
     */
    void clearSelection();

    /**
     * @brief Update fields from item without emitting signals.
     * Used when item is updated externally (e.g., undo/redo).
     */
    void updateFromItem(const TakeoffItem* item);

    /**
     * @brief Update the computed values display.
     * @param wLbPerFt Weight per foot for the assigned shape
     * @param pricePerLb Price per pound for material cost
     */
    void updateComputedValues(double wLbPerFt, double pricePerLb);

    /**
     * @brief Focus the designation field and select all text.
     * Called after a new item is created.
     */
    void focusDesignationField();

    /**
     * @brief Set the list of designations for autocomplete.
     */
    void setDesignationList(const QStringList& designations);

signals:
    // Emitted when user changes designation
    void designationChanged(int itemId, const QString& oldValue, const QString& newValue,
                           int oldShapeId, int newShapeId);
    
    // Emitted when user changes qty
    void qtyChanged(int itemId, int oldValue, int newValue);
    
    // Emitted when user changes notes
    void notesChanged(int itemId, const QString& oldValue, const QString& newValue);
    
    // Emitted when user clicks Pick button
    void pickShapeRequested(int itemId);

private slots:
    void onDesignationEditingFinished();
    void onDesignationSelected(const QString& text);
    void onQtyValueChanged(int value);
    void onNotesChanged();
    void onPickShapeClicked();

private:
    void setupUi();
    void setFieldsEnabled(bool enabled);

    // Container widget
    QWidget* m_container;
    QFormLayout* m_formLayout;

    // "No selection" label
    QLabel* m_noSelectionLabel;

    // Fields
    QLineEdit* m_designationEdit;
    QCompleter* m_designationCompleter;
    QStringListModel* m_designationModel;
    QPushButton* m_pickShapeButton;
    QSpinBox* m_qtySpinBox;
    QTextEdit* m_notesEdit;

    // Info label showing measurement type and length
    QLabel* m_infoLabel;

    // Computed values
    QLabel* m_lengthLabel;
    QLabel* m_weightLabel;
    QLabel* m_costLabel;

    // Current item ID (-1 if none)
    int m_currentItemId;
    int m_currentShapeId;

    // Cached values for detecting changes
    QString m_cachedDesignation;
    int m_cachedQty;
    QString m_cachedNotes;
    double m_cachedLengthFeet;

    // Flag to prevent signal emission during programmatic updates
    bool m_updatingFields;
};

#endif // PROPERTIESDOCK_H
