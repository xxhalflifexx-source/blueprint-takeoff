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
#include "Project.h"
#include "UndoCommands.h"
#include "PdfRenderer.h"
#include "ShapesDatabase.h"

/**
 * @brief Main application window for the Blueprint Takeoff MVP.
 * 
 * Contains the menu bar, toolbar, pages panel, measurement panel, blueprint view,
 * properties dock, and quote summary dock.
 * Manages project save/load, multi-page support, and undo/redo functionality.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    // Internal methods used by undo commands (public so commands can access them)
    void addMeasurementInternal(const Measurement& measurement);
    void removeMeasurementInternal(int measurementId);
    void setMeasurementFieldInternal(int measurementId, MeasurementField field, const QVariant& value);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    // File menu
    void onNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveProjectAs();
    void onAddImagePage();
    void onAddPdf();
    void onImportAiscShapes();
    void onExit();
    
    // Edit menu
    void onUndo();
    void onRedo();
    void onDeleteMeasurement();

    // Tool actions
    void onToolNone();
    void onToolCalibrate();
    void onToolLine();
    void onToolPolyline();

    // View signals
    void onCalibrationCompleted(double pixelsPerInch);
    void onMeasurementCompleted(const Measurement& measurement);
    void onLiveMeasurementChanged(double inches);
    void onMeasurementSelected(int measurementId);
    void onToolCancelled();

    // Pages panel signals
    void onPageSelected(const QString& pageId);
    void onPageDeleteRequested(const QString& pageId);
    void onDeletePage();

    // Undo stack
    void onUndoStackCleanChanged(bool clean);

    // Properties dock signals
    void onPropertyNameChanged(int id, const QString& oldVal, const QString& newVal);
    void onPropertyNotesChanged(int id, const QString& oldVal, const QString& newVal);
    void onPropertyCategoryChanged(int id, Category oldVal, Category newVal);
    void onPropertyMaterialTypeChanged(int id, MaterialType oldVal, MaterialType newVal);
    void onPropertySizeChanged(int id, const QString& oldVal, const QString& newVal);
    void onPropertyLaborClassChanged(int id, LaborClass oldVal, LaborClass newVal);
    void onPickShapeRequested(int measurementId);

    // Quote dock signals
    void onQuoteRatesChanged(const QuoteRates& rates);
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
    void setDirty(bool dirty);
    bool maybeSave();
    bool saveProject(const QString& filePath);
    bool loadProject(const QString& filePath);
    void clearProject();
    void syncProjectFromView();
    void syncViewFromProject();
    void updateQuoteSummary();
    void updatePropertiesPanel();
    void loadCurrentPage();
    void updateMeasurementPanelForPage();

    // UI Components
    BlueprintView* m_blueprintView;
    PagesPanel* m_pagesPanel;
    MeasurementPanel* m_measurementPanel;
    PropertiesDock* m_propertiesDock;
    QuoteDock* m_quoteDock;
    QSplitter* m_mainSplitter;
    QSplitter* m_leftSplitter;
    QToolBar* m_toolBar;

    // PDF Renderer
    PdfRenderer m_pdfRenderer;

    // AISC Shapes Database
    ShapesDatabase m_shapesDb;

    // File menu actions
    QAction* m_newProjectAction;
    QAction* m_openProjectAction;
    QAction* m_saveProjectAction;
    QAction* m_saveProjectAsAction;
    QAction* m_addImagePageAction;
    QAction* m_addPdfAction;
    QAction* m_importAiscAction;
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
    QString m_currentProjectPath;
    bool m_dirty;

    // Current page and measurement selection
    QString m_currentPageId;
    int m_selectedMeasurementId;
};

#endif // MAINWINDOW_H
