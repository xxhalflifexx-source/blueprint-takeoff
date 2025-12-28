#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include <QVariant>
#include "Measurement.h"

// Forward declarations
class MainWindow;

/**
 * @brief Field identifiers for measurement properties.
 */
enum class MeasurementField
{
    Name,
    Notes,
    Category,
    MaterialType,
    Size,
    LaborClass,
    ShapeId,
    ShapeLabel
};

/**
 * @brief Undo command for adding a measurement.
 * 
 * Undo removes the measurement, redo adds it back.
 */
class AddMeasurementCommand : public QUndoCommand
{
public:
    AddMeasurementCommand(MainWindow* mainWindow, const Measurement& measurement,
                          QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    MainWindow* m_mainWindow;
    Measurement m_measurement;
    bool m_firstRedo;  // Skip first redo since measurement is already added
};

/**
 * @brief Undo command for deleting a measurement.
 * 
 * Undo restores the measurement, redo removes it again.
 */
class DeleteMeasurementCommand : public QUndoCommand
{
public:
    DeleteMeasurementCommand(MainWindow* mainWindow, const Measurement& measurement,
                             QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    MainWindow* m_mainWindow;
    Measurement m_measurement;
    bool m_firstRedo;  // Skip first redo since measurement is already deleted
};

/**
 * @brief Undo command for changing a measurement property field.
 * 
 * Supports name, notes, category, materialType, size, laborClass.
 */
class SetMeasurementFieldCommand : public QUndoCommand
{
public:
    SetMeasurementFieldCommand(MainWindow* mainWindow,
                               int measurementId,
                               MeasurementField field,
                               const QVariant& oldValue,
                               const QVariant& newValue,
                               QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

    // Allow merging consecutive edits to the same field
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    void applyValue(const QVariant& value);
    QString fieldName() const;

    MainWindow* m_mainWindow;
    int m_measurementId;
    MeasurementField m_field;
    QVariant m_oldValue;
    QVariant m_newValue;
    bool m_firstRedo;
};

#endif // UNDOCOMMANDS_H
