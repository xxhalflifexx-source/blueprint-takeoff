#include "UndoCommands.h"
#include "MainWindow.h"

// ============================================================================
// AddTakeoffItemCommand
// ============================================================================

AddTakeoffItemCommand::AddTakeoffItemCommand(MainWindow* mainWindow,
                                             const TakeoffItem& item,
                                             QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_mainWindow(mainWindow)
    , m_item(item)
    , m_firstRedo(true)
{
    setText(QString("Add %1").arg(item.kindString()));
}

void AddTakeoffItemCommand::undo()
{
    m_mainWindow->removeTakeoffItemInternal(m_item.id());
}

void AddTakeoffItemCommand::redo()
{
    if (m_firstRedo) {
        m_firstRedo = false;
        return;
    }
    m_mainWindow->addTakeoffItemInternal(m_item);
}

// ============================================================================
// DeleteTakeoffItemCommand
// ============================================================================

DeleteTakeoffItemCommand::DeleteTakeoffItemCommand(MainWindow* mainWindow,
                                                   const TakeoffItem& item,
                                                   QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_mainWindow(mainWindow)
    , m_item(item)
    , m_firstRedo(true)
{
    setText(QString("Delete %1").arg(item.kindString()));
}

void DeleteTakeoffItemCommand::undo()
{
    m_mainWindow->addTakeoffItemInternal(m_item);
}

void DeleteTakeoffItemCommand::redo()
{
    if (m_firstRedo) {
        m_firstRedo = false;
        return;
    }
    m_mainWindow->removeTakeoffItemInternal(m_item.id());
}

// ============================================================================
// SetTakeoffItemFieldCommand
// ============================================================================

SetTakeoffItemFieldCommand::SetTakeoffItemFieldCommand(MainWindow* mainWindow,
                                                       int itemId,
                                                       TakeoffItemField field,
                                                       const QVariant& oldValue,
                                                       const QVariant& newValue,
                                                       QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_mainWindow(mainWindow)
    , m_itemId(itemId)
    , m_field(field)
    , m_oldValue(oldValue)
    , m_newValue(newValue)
    , m_firstRedo(true)
{
    setText(QString("Set %1").arg(fieldName()));
}

void SetTakeoffItemFieldCommand::undo()
{
    applyValue(m_oldValue);
}

void SetTakeoffItemFieldCommand::redo()
{
    if (m_firstRedo) {
        m_firstRedo = false;
        return;
    }
    applyValue(m_newValue);
}

int SetTakeoffItemFieldCommand::id() const
{
    return 1000 + m_itemId * 10 + static_cast<int>(m_field);
}

bool SetTakeoffItemFieldCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) {
        return false;
    }

    const SetTakeoffItemFieldCommand* cmd = 
        static_cast<const SetTakeoffItemFieldCommand*>(other);

    if (cmd->m_itemId != m_itemId || cmd->m_field != m_field) {
        return false;
    }

    m_newValue = cmd->m_newValue;
    return true;
}

void SetTakeoffItemFieldCommand::applyValue(const QVariant& value)
{
    m_mainWindow->setTakeoffItemFieldInternal(m_itemId, m_field, value);
}

QString SetTakeoffItemFieldCommand::fieldName() const
{
    switch (m_field) {
        case TakeoffItemField::Designation: return "Designation";
        case TakeoffItemField::Qty:         return "Qty";
        case TakeoffItemField::Notes:       return "Notes";
        case TakeoffItemField::ShapeId:     return "Shape";
        default:                            return "Field";
    }
}
