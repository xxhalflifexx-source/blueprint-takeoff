#ifndef PROPERTIESDOCK_H
#define PROPERTIESDOCK_H

#include <QDockWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QWidget>
#include <QPushButton>

#include "Measurement.h"

/**
 * @brief Dock widget for editing measurement properties.
 * 
 * Shows editable fields for the selected measurement:
 * Name, Notes, Category, Material Type, Size, Labor Class.
 */
class PropertiesDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit PropertiesDock(QWidget* parent = nullptr);
    ~PropertiesDock();

    /**
     * @brief Set the measurement to display/edit.
     * @param measurement Pointer to measurement (nullptr to clear)
     * @param id Measurement ID (for signal emission)
     */
    void setMeasurement(const Measurement* measurement, int id);

    /**
     * @brief Clear the properties panel (no selection).
     */
    void clearSelection();

    /**
     * @brief Update fields from measurement without emitting signals.
     * Used when measurement is updated externally (e.g., undo/redo).
     */
    void updateFromMeasurement(const Measurement* measurement);

    /**
     * @brief Focus the size field and select all text.
     * Called after a new measurement is created.
     */
    void focusSizeField();

signals:
    // Emitted when user changes a field
    void nameChanged(int measurementId, const QString& oldValue, const QString& newValue);
    void notesChanged(int measurementId, const QString& oldValue, const QString& newValue);
    void categoryChanged(int measurementId, Category oldValue, Category newValue);
    void materialTypeChanged(int measurementId, MaterialType oldValue, MaterialType newValue);
    void sizeChanged(int measurementId, const QString& oldValue, const QString& newValue);
    void laborClassChanged(int measurementId, LaborClass oldValue, LaborClass newValue);
    
    // Emitted when user clicks Pick AISC Shape button
    void pickShapeRequested(int measurementId);

private slots:
    void onNameEditingFinished();
    void onNotesChanged();
    void onCategoryChanged(int index);
    void onMaterialTypeChanged(int index);
    void onSizeEditingFinished();
    void onLaborClassChanged(int index);
    void onPickShapeClicked();

private:
    void setupUi();
    void populateComboBoxes();
    void setFieldsEnabled(bool enabled);

    // Container widget
    QWidget* m_container;
    QFormLayout* m_formLayout;

    // "No selection" label
    QLabel* m_noSelectionLabel;

    // Fields
    QLineEdit* m_nameEdit;
    QTextEdit* m_notesEdit;
    QComboBox* m_categoryCombo;
    QComboBox* m_materialTypeCombo;
    QLineEdit* m_sizeEdit;
    QPushButton* m_pickShapeButton;
    QComboBox* m_laborClassCombo;

    // Info label showing measurement type and length
    QLabel* m_infoLabel;

    // Current measurement ID (-1 if none)
    int m_currentMeasurementId;

    // Cached values for detecting changes
    QString m_cachedName;
    QString m_cachedNotes;
    Category m_cachedCategory;
    MaterialType m_cachedMaterialType;
    QString m_cachedSize;
    LaborClass m_cachedLaborClass;

    // Flag to prevent signal emission during programmatic updates
    bool m_updatingFields;
};

#endif // PROPERTIESDOCK_H

