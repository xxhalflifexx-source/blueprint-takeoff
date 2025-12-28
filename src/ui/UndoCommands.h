#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include <QVariant>
#include "../models/TakeoffItem.h"

// Forward declarations
class MainWindow;

/**
 * @brief Field identifiers for takeoff item properties.
 */
enum class TakeoffItemField
{
    Designation,
    Qty,
    Notes,
    ShapeId
};

/**
 * @brief Undo command for adding a takeoff item.
 */
class AddTakeoffItemCommand : public QUndoCommand
{
public:
    AddTakeoffItemCommand(MainWindow* mainWindow, const TakeoffItem& item,
                          QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    MainWindow* m_mainWindow;
    TakeoffItem m_item;
    bool m_firstRedo;
};

/**
 * @brief Undo command for deleting a takeoff item.
 */
class DeleteTakeoffItemCommand : public QUndoCommand
{
public:
    DeleteTakeoffItemCommand(MainWindow* mainWindow, const TakeoffItem& item,
                             QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

private:
    MainWindow* m_mainWindow;
    TakeoffItem m_item;
    bool m_firstRedo;
};

/**
 * @brief Undo command for changing a takeoff item property field.
 */
class SetTakeoffItemFieldCommand : public QUndoCommand
{
public:
    SetTakeoffItemFieldCommand(MainWindow* mainWindow,
                               int itemId,
                               TakeoffItemField field,
                               const QVariant& oldValue,
                               const QVariant& newValue,
                               QUndoCommand* parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    void applyValue(const QVariant& value);
    QString fieldName() const;

    MainWindow* m_mainWindow;
    int m_itemId;
    TakeoffItemField m_field;
    QVariant m_oldValue;
    QVariant m_newValue;
    bool m_firstRedo;
};

#endif // UNDOCOMMANDS_H
