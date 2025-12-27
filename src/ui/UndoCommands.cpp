#include "UndoCommands.h"
#include "MainWindow.h"

// ============================================================================
// AddMeasurementCommand
// ============================================================================

AddMeasurementCommand::AddMeasurementCommand(MainWindow* mainWindow,
                                             const Measurement& measurement,
                                             QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_mainWindow(mainWindow)
    , m_measurement(measurement)
    , m_firstRedo(true)
{
    setText(QString("Add %1").arg(measurement.typeString()));
}

void AddMeasurementCommand::undo()
{
    m_mainWindow->removeMeasurementInternal(m_measurement.id());
}

void AddMeasurementCommand::redo()
{
    // Skip first redo - the measurement was already added when the command was created
    if (m_firstRedo) {
        m_firstRedo = false;
        return;
    }
    m_mainWindow->addMeasurementInternal(m_measurement);
}

// ============================================================================
// DeleteMeasurementCommand
// ============================================================================

DeleteMeasurementCommand::DeleteMeasurementCommand(MainWindow* mainWindow,
                                                   const Measurement& measurement,
                                                   QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_mainWindow(mainWindow)
    , m_measurement(measurement)
    , m_firstRedo(true)
{
    setText(QString("Delete %1").arg(measurement.typeString()));
}

void DeleteMeasurementCommand::undo()
{
    m_mainWindow->addMeasurementInternal(m_measurement);
}

void DeleteMeasurementCommand::redo()
{
    // Skip first redo - the measurement was already deleted when the command was created
    if (m_firstRedo) {
        m_firstRedo = false;
        return;
    }
    m_mainWindow->removeMeasurementInternal(m_measurement.id());
}

// ============================================================================
// SetMeasurementFieldCommand
// ============================================================================

SetMeasurementFieldCommand::SetMeasurementFieldCommand(MainWindow* mainWindow,
                                                       int measurementId,
                                                       MeasurementField field,
                                                       const QVariant& oldValue,
                                                       const QVariant& newValue,
                                                       QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_mainWindow(mainWindow)
    , m_measurementId(measurementId)
    , m_field(field)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
    , m_firstRedo(true)
{
    setText(QString("Set %1").arg(fieldName()));
}

void SetMeasurementFieldCommand::undo()
{
    applyValue(m_oldValue);
}

void SetMeasurementFieldCommand::redo()
{
    // Skip first redo - the value was already changed when the command was created
    if (m_firstRedo) {
        m_firstRedo = false;
        return;
    }
    applyValue(m_newValue);
}

int SetMeasurementFieldCommand::id() const
{
    // Unique ID for merging: combine measurement ID and field
    return 1000 + m_measurementId * 10 + static_cast<int>(m_field);
}

bool SetMeasurementFieldCommand::mergeWith(const QUndoCommand* other)
{
    // Only merge with same command type
    if (other->id() != id()) {
        return false;
    }

    const SetMeasurementFieldCommand* cmd = 
        static_cast<const SetMeasurementFieldCommand*>(other);

    // Same measurement and field?
    if (cmd->m_measurementId != m_measurementId || cmd->m_field != m_field) {
        return false;
    }

    // Merge: keep our old value, take their new value
    m_newValue = cmd->m_newValue;
    return true;
}

void SetMeasurementFieldCommand::applyValue(const QVariant& value)
{
    m_mainWindow->setMeasurementFieldInternal(m_measurementId, m_field, value);
}

QString SetMeasurementFieldCommand::fieldName() const
{
    switch (m_field) {
        case MeasurementField::Name:         return "Name";
        case MeasurementField::Notes:        return "Notes";
        case MeasurementField::Category:     return "Category";
        case MeasurementField::MaterialType: return "Material Type";
        case MeasurementField::Size:         return "Size";
        case MeasurementField::LaborClass:   return "Labor Class";
        default:                             return "Field";
    }
}
