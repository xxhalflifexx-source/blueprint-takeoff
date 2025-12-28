#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QAction>
#include <QActionGroup>
#include <QUndoStack>
#include <QCloseEvent>
#include <QVariant>

#include "BlueprintView.h"
#include "MeasurementPanel.h"
#include "PagesPanel.h"
#include "PropertiesDock.h"
#include "QuoteDock.h"
#include "../models/Project.h"
#include "../models/TakeoffItem.h"
#include "UndoCommands.h"
#include "PdfRenderer.h"

/**
 * @brief Main application window for the Blueprint Takeoff MVP.
 * 
 * Uses SQLite for project persistence (.takeoff.db files).
 * Manages pages, takeoff items, and quote calculations.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Internal methods used by undo commands
    void addTakeoffItemInternal(TakeoffItem& item);
    void removeTakeoffItemInternal(int itemId);
    void setTakeoffItemFieldInternal(int itemId, TakeoffItemField field, const QVariant& value);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // File menu
    void onNewProject();
    void onOpenProject();
    void onAddImagePage();
    void onAddPdf();
    void onImportShapes();
    void onExit();
    
    // Edit menu
    void onUndo();
    void onRedo();
    void onDeleteItem();

    // Tool actions
    void onToolNone();
    void onToolCalibrate();
    void onToolLine();
    void onToolPolyline();

    // View signals
    void onCalibrationCompleted(double pixelsPerInch);
    void onMeasurementCompleted(const Measurement& measurement);
    void onLiveMeasurementChanged(double inches);
    void onItemSelected(int itemId);
    void onToolCancelled();

    // Pages panel signals
    void onPageSelected(const QString& pageId);
    void onPageDeleteRequested(const QString& pageId);
    void onDeletePage();

    // Undo stack
    void onUndoStackCleanChanged(bool clean);

    // Properties dock signals
    void onDesignationChanged(int itemId, const QString& oldVal, const QString& newVal,
                             int oldShapeId, int newShapeId);
    void onQtyChanged(int itemId, int oldVal, int newVal);
    void onNotesChanged(int itemId, const QString& oldVal, const QString& newVal);
    void onPickShapeRequested(int itemId);

    // Quote dock signals
    void onMaterialPriceChanged(double pricePerLb);
    void onCurrentPageOnlyChanged(bool currentPageOnly);

private:
    void setupUi();
    void createMenuBar();
    void createToolBar();
    void createStatusBar();
    void createCentralWidget();
    void createDockWidgets();
    void connectSignals();
    void updateStatusBar(const QString& message);
    void updateWindowTitle();
    bool maybeSave();
    void clearProject();
    void updateQuoteSummary();
    void updatePropertiesPanel();
    void loadCurrentPage();
    void updateItemsPanelForPage();
    void refreshDesignationAutocomplete();
    void updateItemDisplay(int itemId);

    // UI Components
    BlueprintView* m_blueprintView;
    PagesPanel* m_pagesPanel;
    MeasurementPanel* m_itemsPanel;  // Renamed from measurementPanel
    PropertiesDock* m_propertiesDock;
    QuoteDock* m_quoteDock;
    QSplitter* m_mainSplitter;
    QSplitter* m_leftSplitter;
    QToolBar* m_toolBar;

    // PDF Renderer
    PdfRenderer m_pdfRenderer;

    // File menu actions
    QAction* m_newProjectAction;
    QAction* m_openProjectAction;
    QAction* m_addImagePageAction;
    QAction* m_addPdfAction;
    QAction* m_importShapesAction;
    QAction* m_exitAction;

    // Edit menu actions
    QAction* m_undoAction;
    QAction* m_redoAction;
    QAction* m_deleteAction;
    QAction* m_deletePageAction;

    // Tool actions
    QAction* m_noneToolAction;
    QAction* m_calibrateAction;
    QAction* m_lineAction;
    QAction* m_polylineAction;
    QActionGroup* m_toolGroup;

    // Undo/Redo
    QUndoStack* m_undoStack;

    // Project data
    Project m_project;

    // Current page and item selection
    QString m_currentPageId;
    int m_selectedItemId;
};

#endif // MAINWINDOW_H
