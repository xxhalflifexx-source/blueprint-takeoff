#include "MainWindow.h"
#include "UndoCommands.h"
#include "PdfImportDialog.h"
#include "ShapePickerDialog.h"

#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>
#include <QFileInfo>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_blueprintView(nullptr)
    , m_pagesPanel(nullptr)
    , m_measurementPanel(nullptr)
    , m_propertiesDock(nullptr)
    , m_quoteDock(nullptr)
    , m_mainSplitter(nullptr)
    , m_leftSplitter(nullptr)
    , m_toolBar(nullptr)
    , m_newProjectAction(nullptr)
    , m_openProjectAction(nullptr)
    , m_saveProjectAction(nullptr)
    , m_saveProjectAsAction(nullptr)
    , m_addImagePageAction(nullptr)
    , m_addPdfAction(nullptr)
    , m_importAiscAction(nullptr)
    , m_exitAction(nullptr)
    , m_undoAction(nullptr)
    , m_redoAction(nullptr)
    , m_deleteAction(nullptr)
    , m_deletePageAction(nullptr)
    , m_noneToolAction(nullptr)
    , m_calibrateAction(nullptr)
    , m_lineAction(nullptr)
    , m_polylineAction(nullptr)
    , m_toolGroup(nullptr)
    , m_undoStack(nullptr)
    , m_currentProjectPath()
    , m_dirty(false)
    , m_currentPageId()
    , m_selectedMeasurementId(-1)
{
    m_undoStack = new QUndoStack(this);
    
    // Open shapes database
    m_shapesDb.open();
    
    setupUi();
    connectSignals();
    updateWindowTitle();
    resize(1400, 900);
}

MainWindow::~MainWindow()
{
    // Qt handles child deletion
}

void MainWindow::setupUi()
{
    createMenuBar();
    createToolBar();
    createStatusBar();
    createCentralWidget();
    createDockWidgets();
}

void MainWindow::createMenuBar()
{
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    // === File menu ===
    QMenu* fileMenu = menuBar->addMenu("&File");

    m_newProjectAction = new QAction("&New Project", this);
    m_newProjectAction->setShortcut(QKeySequence::New);
    m_newProjectAction->setStatusTip("Create a new project");
    fileMenu->addAction(m_newProjectAction);

    m_openProjectAction = new QAction("&Open Project...", this);
    m_openProjectAction->setShortcut(QKeySequence::Open);
    m_openProjectAction->setStatusTip("Open an existing project file");
    fileMenu->addAction(m_openProjectAction);

    fileMenu->addSeparator();

    m_saveProjectAction = new QAction("&Save Project", this);
    m_saveProjectAction->setShortcut(QKeySequence::Save);
    m_saveProjectAction->setStatusTip("Save the current project");
    fileMenu->addAction(m_saveProjectAction);

    m_saveProjectAsAction = new QAction("Save Project &As...", this);
    m_saveProjectAsAction->setShortcut(QKeySequence::SaveAs);
    m_saveProjectAsAction->setStatusTip("Save the project with a new name");
    fileMenu->addAction(m_saveProjectAsAction);

    fileMenu->addSeparator();

    m_addImagePageAction = new QAction("Add &Image Page...", this);
    m_addImagePageAction->setStatusTip("Add an image (PNG/JPG) as a new page");
    fileMenu->addAction(m_addImagePageAction);

    m_addPdfAction = new QAction("Add P&DF...", this);
    m_addPdfAction->setStatusTip("Add pages from a PDF file");
    fileMenu->addAction(m_addPdfAction);

    fileMenu->addSeparator();

    m_importAiscAction = new QAction("Import AISC &Shapes...", this);
    m_importAiscAction->setStatusTip("Import AISC shapes from an XLSX or CSV file");
    fileMenu->addAction(m_importAiscAction);

    fileMenu->addSeparator();

    m_exitAction = new QAction("E&xit", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip("Exit the application");
    fileMenu->addAction(m_exitAction);

    // === Edit menu ===
    QMenu* editMenu = menuBar->addMenu("&Edit");

    m_undoAction = m_undoStack->createUndoAction(this, "&Undo");
    m_undoAction->setShortcut(QKeySequence::Undo);
    editMenu->addAction(m_undoAction);

    m_redoAction = m_undoStack->createRedoAction(this, "&Redo");
    m_redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addAction(m_redoAction);

    editMenu->addSeparator();

    m_deleteAction = new QAction("&Delete Measurement", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setStatusTip("Delete the selected measurement");
    m_deleteAction->setEnabled(false);
    editMenu->addAction(m_deleteAction);

    m_deletePageAction = new QAction("Delete &Page", this);
    m_deletePageAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete));
    m_deletePageAction->setStatusTip("Delete the selected page");
    m_deletePageAction->setEnabled(false);
    editMenu->addAction(m_deletePageAction);
}

void MainWindow::createToolBar()
{
    m_toolBar = new QToolBar("Tools", this);
    m_toolBar->setMovable(false);
    addToolBar(Qt::TopToolBarArea, m_toolBar);

    // Add save/open buttons to toolbar
    m_toolBar->addAction(m_openProjectAction);
    m_toolBar->addAction(m_saveProjectAction);
    m_toolBar->addSeparator();

    // Create action group for exclusive tool selection
    m_toolGroup = new QActionGroup(this);
    m_toolGroup->setExclusive(true);

    // Pan/Select tool (no measurement)
    m_noneToolAction = new QAction("Pan", this);
    m_noneToolAction->setCheckable(true);
    m_noneToolAction->setChecked(true);
    m_noneToolAction->setStatusTip("Pan and zoom mode (no measurement)");
    m_toolGroup->addAction(m_noneToolAction);
    m_toolBar->addAction(m_noneToolAction);

    m_toolBar->addSeparator();

    // Calibrate tool
    m_calibrateAction = new QAction("Calibrate", this);
    m_calibrateAction->setCheckable(true);
    m_calibrateAction->setStatusTip("Calibrate: click two points and enter the real distance");
    m_toolGroup->addAction(m_calibrateAction);
    m_toolBar->addAction(m_calibrateAction);

    m_toolBar->addSeparator();

    // Line tool
    m_lineAction = new QAction("Line", this);
    m_lineAction->setCheckable(true);
    m_lineAction->setStatusTip("Measure a line: click two points");
    m_toolGroup->addAction(m_lineAction);
    m_toolBar->addAction(m_lineAction);

    // Polyline tool
    m_polylineAction = new QAction("Polyline", this);
    m_polylineAction->setCheckable(true);
    m_polylineAction->setStatusTip("Measure a polyline: click points, double-click to finish");
    m_toolGroup->addAction(m_polylineAction);
    m_toolBar->addAction(m_polylineAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage("Ready. Create a new project or open an existing one.");
}

void MainWindow::createCentralWidget()
{
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Left panel: Pages + Measurements in vertical splitter
    m_leftSplitter = new QSplitter(Qt::Vertical, m_mainSplitter);
    
    // Pages panel
    m_pagesPanel = new PagesPanel(m_leftSplitter);
    m_leftSplitter->addWidget(m_pagesPanel);
    
    // Measurements panel
    m_measurementPanel = new MeasurementPanel(m_leftSplitter);
    m_leftSplitter->addWidget(m_measurementPanel);
    
    // Set left splitter sizes
    m_leftSplitter->setSizes({200, 400});
    
    m_mainSplitter->addWidget(m_leftSplitter);

    // Center: Blueprint view
    m_blueprintView = new BlueprintView(m_mainSplitter);
    m_mainSplitter->addWidget(m_blueprintView);

    // Set initial sizes (left panel narrower)
    m_mainSplitter->setSizes({250, 1000});

    setCentralWidget(m_mainSplitter);
}

void MainWindow::createDockWidgets()
{
    // Properties dock (right side)
    m_propertiesDock = new PropertiesDock(this);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);

    // Quote dock (bottom)
    m_quoteDock = new QuoteDock(this);
    addDockWidget(Qt::BottomDockWidgetArea, m_quoteDock);
}

void MainWindow::connectSignals()
{
    // File menu actions
    connect(m_newProjectAction, &QAction::triggered, this, &MainWindow::onNewProject);
    connect(m_openProjectAction, &QAction::triggered, this, &MainWindow::onOpenProject);
    connect(m_saveProjectAction, &QAction::triggered, this, &MainWindow::onSaveProject);
    connect(m_saveProjectAsAction, &QAction::triggered, this, &MainWindow::onSaveProjectAs);
    connect(m_addImagePageAction, &QAction::triggered, this, &MainWindow::onAddImagePage);
    connect(m_addPdfAction, &QAction::triggered, this, &MainWindow::onAddPdf);
    connect(m_importAiscAction, &QAction::triggered, this, &MainWindow::onImportAiscShapes);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExit);

    // Edit menu actions
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteMeasurement);
    connect(m_deletePageAction, &QAction::triggered, this, &MainWindow::onDeletePage);

    // Tool actions
    connect(m_noneToolAction, &QAction::triggered, this, &MainWindow::onToolNone);
    connect(m_calibrateAction, &QAction::triggered, this, &MainWindow::onToolCalibrate);
    connect(m_lineAction, &QAction::triggered, this, &MainWindow::onToolLine);
    connect(m_polylineAction, &QAction::triggered, this, &MainWindow::onToolPolyline);

    // Blueprint view signals
    connect(m_blueprintView, &BlueprintView::calibrationCompleted,
            this, &MainWindow::onCalibrationCompleted);
    connect(m_blueprintView, &BlueprintView::measurementCompleted,
            this, &MainWindow::onMeasurementCompleted);
    connect(m_blueprintView, &BlueprintView::liveMeasurementChanged,
            this, &MainWindow::onLiveMeasurementChanged);
    connect(m_blueprintView, &BlueprintView::toolCancelled,
            this, &MainWindow::onToolCancelled);

    // Pages panel signals
    connect(m_pagesPanel, &PagesPanel::pageSelected,
            this, &MainWindow::onPageSelected);
    connect(m_pagesPanel, &PagesPanel::pageDeleteRequested,
            this, &MainWindow::onPageDeleteRequested);

    // Measurement panel signals
    connect(m_measurementPanel, &MeasurementPanel::measurementSelected,
            this, &MainWindow::onMeasurementSelected);

    // Undo stack signals
    connect(m_undoStack, &QUndoStack::cleanChanged,
            this, &MainWindow::onUndoStackCleanChanged);

    // Properties dock signals
    connect(m_propertiesDock, &PropertiesDock::nameChanged,
            this, &MainWindow::onPropertyNameChanged);
    connect(m_propertiesDock, &PropertiesDock::notesChanged,
            this, &MainWindow::onPropertyNotesChanged);
    connect(m_propertiesDock, &PropertiesDock::categoryChanged,
            this, &MainWindow::onPropertyCategoryChanged);
    connect(m_propertiesDock, &PropertiesDock::materialTypeChanged,
            this, &MainWindow::onPropertyMaterialTypeChanged);
    connect(m_propertiesDock, &PropertiesDock::sizeChanged,
            this, &MainWindow::onPropertySizeChanged);
    connect(m_propertiesDock, &PropertiesDock::laborClassChanged,
            this, &MainWindow::onPropertyLaborClassChanged);
    connect(m_propertiesDock, &PropertiesDock::pickShapeRequested,
            this, &MainWindow::onPickShapeRequested);

    // Quote dock signals
    connect(m_quoteDock, &QuoteDock::ratesChanged,
            this, &MainWindow::onQuoteRatesChanged);
    connect(m_quoteDock, &QuoteDock::currentPageOnlyChanged,
            this, &MainWindow::onCurrentPageOnlyChanged);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

// ============================================================================
// File Menu Slots
// ============================================================================

void MainWindow::onNewProject()
{
    if (!maybeSave()) {
        return;
    }

    clearProject();
    updateStatusBar("New project created. Add an image or PDF to begin.");
}

void MainWindow::onOpenProject()
{
    if (!maybeSave()) {
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open Project",
        QString(),
        Project::FILE_FILTER
    );

    if (filePath.isEmpty()) {
        return;
    }

    if (loadProject(filePath)) {
        updateStatusBar(QString("Project loaded: %1").arg(filePath));
    }
}

void MainWindow::onSaveProject()
{
    if (m_currentProjectPath.isEmpty()) {
        onSaveProjectAs();
    } else {
        saveProject(m_currentProjectPath);
    }
}

void MainWindow::onSaveProjectAs()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Project As",
        QString(),
        Project::FILE_FILTER
    );

    if (filePath.isEmpty()) {
        return;
    }

    // Ensure proper extension
    if (!filePath.endsWith(Project::FILE_EXTENSION)) {
        filePath += Project::FILE_EXTENSION;
    }

    if (saveProject(filePath)) {
        m_currentProjectPath = filePath;
        updateWindowTitle();
        updateStatusBar(QString("Project saved: %1").arg(filePath));
    }
}

void MainWindow::onAddImagePage()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Add Image Page",
        QString(),
        "Images (*.png *.jpg *.jpeg);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // Create a new image page
    Page page = Page::createImagePage(filePath);
    m_project.addPage(page);
    m_pagesPanel->addPage(page);
    
    setDirty(true);
    
    // Select the new page
    m_pagesPanel->selectPage(page.id());
    
    updateStatusBar(QString("Added page: %1. Calibrate before measuring.").arg(page.listDisplayString()));
}

void MainWindow::onAddPdf()
{
    // Check if PDF support is available
    if (!PdfRenderer::isAvailable()) {
        QMessageBox::information(this, "PDF Support Not Available",
            "PDF support requires the Qt PDF module which is not installed.\n\n"
            "To enable PDF support, install Qt PDF via the Qt Maintenance Tool.\n\n"
            "For now, you can convert your PDF pages to PNG/JPG images and use 'Add Image Page'.");
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Add PDF",
        QString(),
        "PDF Files (*.pdf);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // Try to open the PDF
    if (!m_pdfRenderer.openPdf(filePath)) {
        QMessageBox::warning(this, "Error", 
            QString("Failed to open PDF: %1").arg(m_pdfRenderer.lastError()));
        return;
    }

    int totalPages = m_pdfRenderer.pageCount();
    if (totalPages == 0) {
        QMessageBox::warning(this, "Error", "PDF has no pages.");
        m_pdfRenderer.close();
        return;
    }

    // Show import dialog
    PdfImportDialog dialog(filePath, totalPages, this);
    if (dialog.exec() != QDialog::Accepted) {
        m_pdfRenderer.close();
        return;
    }

    int fromPage = dialog.fromPage();
    int toPage = dialog.toPage();

    // Import selected pages
    QString firstPageId;
    for (int i = fromPage; i <= toPage; ++i) {
        Page page = Page::createPdfPage(filePath, i - 1, totalPages);  // 0-based index
        m_project.addPage(page);
        m_pagesPanel->addPage(page);
        
        if (firstPageId.isEmpty()) {
            firstPageId = page.id();
        }
    }

    m_pdfRenderer.close();
    setDirty(true);

    // Select the first imported page
    if (!firstPageId.isEmpty()) {
        m_pagesPanel->selectPage(firstPageId);
    }

    int count = toPage - fromPage + 1;
    updateStatusBar(QString("Added %1 page(s) from PDF. Calibrate before measuring.").arg(count));
}

void MainWindow::onImportAiscShapes()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Import AISC Shapes",
        QString(),
        "Excel Files (*.xlsx);;CSV Files (*.csv);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // Make sure database is open
    if (!m_shapesDb.isOpen()) {
        if (!m_shapesDb.open()) {
            QMessageBox::critical(this, "Database Error",
                QString("Failed to open shapes database: %1").arg(m_shapesDb.lastError()));
            return;
        }
    }

    int count = 0;
    if (filePath.endsWith(".xlsx", Qt::CaseInsensitive)) {
        count = m_shapesDb.importFromXlsx(filePath);
    } else {
        count = m_shapesDb.importFromCsv(filePath);
    }

    if (count < 0) {
        QMessageBox::critical(this, "Import Error",
            QString("Failed to import shapes: %1").arg(m_shapesDb.lastError()));
    } else if (count == 0) {
        QMessageBox::warning(this, "Import", "No shapes were imported from the file.");
    } else {
        QMessageBox::information(this, "Import Complete",
            QString("Successfully imported %1 shapes.\n\nTotal shapes in database: %2")
            .arg(count).arg(m_shapesDb.shapeCount()));
    }
}

void MainWindow::onExit()
{
    close();
}

// ============================================================================
// Edit Menu Slots
// ============================================================================

void MainWindow::onUndo()
{
    m_undoStack->undo();
}

void MainWindow::onRedo()
{
    m_undoStack->redo();
}

void MainWindow::onDeleteMeasurement()
{
    int selectedId = m_measurementPanel->selectedMeasurementId();
    if (selectedId < 0) {
        return;
    }

    // Find the measurement
    Measurement* m = m_project.findMeasurement(selectedId);
    if (!m) {
        return;
    }

    // Create undo command
    Measurement copy = *m;
    
    // Remove the measurement
    removeMeasurementInternal(selectedId);
    
    // Push undo command (with firstRedo = true to skip initial redo)
    m_undoStack->push(new DeleteMeasurementCommand(this, copy));
}

void MainWindow::onDeletePage()
{
    if (m_currentPageId.isEmpty()) {
        return;
    }
    onPageDeleteRequested(m_currentPageId);
}

// ============================================================================
// Tool Slots
// ============================================================================

void MainWindow::onToolNone()
{
    m_blueprintView->setTool(Tool::None);
    updateStatusBar("Pan mode. Use mouse wheel to zoom, drag to pan.");
}

void MainWindow::onToolCalibrate()
{
    if (m_currentPageId.isEmpty()) {
        QMessageBox::information(this, "No Page Selected",
            "Please add and select a page first.");
        m_noneToolAction->setChecked(true);
        m_blueprintView->setTool(Tool::None);
        return;
    }
    
    m_blueprintView->setTool(Tool::Calibrate);
    updateStatusBar("Calibrate: Click Point A, then Point B, then enter the real distance.");
}

void MainWindow::onToolLine()
{
    if (m_currentPageId.isEmpty()) {
        QMessageBox::information(this, "No Page Selected",
            "Please add and select a page first.");
        m_noneToolAction->setChecked(true);
        m_blueprintView->setTool(Tool::None);
        return;
    }
    
    // Check calibration for current page
    Page* page = m_project.findPage(m_currentPageId);
    if (!page || !page->calibration().isCalibrated()) {
        QMessageBox::information(this, "Calibration Required",
            "Please calibrate this page first using the Calibrate tool.");
        m_noneToolAction->setChecked(true);
        m_blueprintView->setTool(Tool::None);
        return;
    }
    
    m_blueprintView->setTool(Tool::Line);
    updateStatusBar("Line tool: Click two points to measure distance.");
}

void MainWindow::onToolPolyline()
{
    if (m_currentPageId.isEmpty()) {
        QMessageBox::information(this, "No Page Selected",
            "Please add and select a page first.");
        m_noneToolAction->setChecked(true);
        m_blueprintView->setTool(Tool::None);
        return;
    }
    
    // Check calibration for current page
    Page* page = m_project.findPage(m_currentPageId);
    if (!page || !page->calibration().isCalibrated()) {
        QMessageBox::information(this, "Calibration Required",
            "Please calibrate this page first using the Calibrate tool.");
        m_noneToolAction->setChecked(true);
        m_blueprintView->setTool(Tool::None);
        return;
    }
    
    m_blueprintView->setTool(Tool::Polyline);
    updateStatusBar("Polyline tool: Click points, double-click to finish.");
}

// ============================================================================
// View Signal Slots
// ============================================================================

void MainWindow::onCalibrationCompleted(double pixelsPerInch)
{
    // Sync calibration to current page
    Page* page = m_project.findPage(m_currentPageId);
    if (page) {
        page->calibration() = m_blueprintView->calibration();
    }
    setDirty(true);
    
    updateStatusBar(QString("Calibration complete: %1 pixels/inch. Ready to measure.")
                    .arg(pixelsPerInch, 0, 'f', 2));
    
    // Switch to pan mode after calibration
    m_noneToolAction->setChecked(true);
}

void MainWindow::onMeasurementCompleted(const Measurement& measurement)
{
    // Create a copy and set the pageId
    Measurement m = measurement;
    m.setPageId(m_currentPageId);
    
    // Add to project and panel
    addMeasurementInternal(m);
    
    // Push undo command
    m_undoStack->push(new AddMeasurementCommand(this, m));
    
    // Auto-select the new measurement and focus size field
    m_measurementPanel->selectMeasurement(m.id());
    m_propertiesDock->focusSizeField();
    
    updateStatusBar(QString("Measurement added: %1").arg(m.displayString()));
}

void MainWindow::onLiveMeasurementChanged(double inches)
{
    if (inches > 0.0) {
        updateStatusBar(QString("Current: %1 in").arg(inches, 0, 'f', 2));
    }
}

void MainWindow::onMeasurementSelected(int measurementId)
{
    m_selectedMeasurementId = measurementId;
    m_blueprintView->highlightMeasurement(measurementId);
    m_deleteAction->setEnabled(measurementId >= 0);
    
    // Update properties panel
    updatePropertiesPanel();
    
    if (measurementId >= 0) {
        // Find the measurement and show details
        const Measurement* m = m_project.findMeasurement(measurementId);
        if (m) {
            updateStatusBar(QString("Selected: %1").arg(m->displayString()));
        }
    }
}

void MainWindow::onToolCancelled()
{
    // Switch back to pan mode when Esc is pressed
    m_noneToolAction->setChecked(true);
    m_blueprintView->setTool(Tool::None);
    updateStatusBar("Tool cancelled. Pan mode.");
}

// ============================================================================
// Pages Panel Slots
// ============================================================================

void MainWindow::onPageSelected(const QString& pageId)
{
    if (pageId == m_currentPageId) {
        return;
    }
    
    // Save calibration from current view to the previous page
    if (!m_currentPageId.isEmpty()) {
        Page* prevPage = m_project.findPage(m_currentPageId);
        if (prevPage) {
            prevPage->calibration() = m_blueprintView->calibration();
        }
    }
    
    m_currentPageId = pageId;
    m_selectedMeasurementId = -1;
    
    // Load the new page
    loadCurrentPage();
    
    // Update measurement panel for this page
    updateMeasurementPanelForPage();
    
    // Clear selection
    m_propertiesDock->clearSelection();
    m_deleteAction->setEnabled(false);
    m_deletePageAction->setEnabled(!m_currentPageId.isEmpty());
    
    // Update quote summary
    updateQuoteSummary();
    
    // Reset to pan mode
    m_noneToolAction->setChecked(true);
    m_blueprintView->setTool(Tool::None);
    
    Page* page = m_project.findPage(pageId);
    if (page) {
        if (page->calibration().isCalibrated()) {
            updateStatusBar(QString("Page: %1 (calibrated)").arg(page->listDisplayString()));
        } else {
            updateStatusBar(QString("Page: %1 - Calibrate before measuring.").arg(page->listDisplayString()));
        }
    }
}

void MainWindow::onPageDeleteRequested(const QString& pageId)
{
    Page* page = m_project.findPage(pageId);
    if (!page) {
        return;
    }
    
    // Confirm deletion
    int measurementCount = m_project.measurementsForPage(pageId).size();
    QString message;
    if (measurementCount > 0) {
        message = QString("Delete page '%1' and its %2 measurement(s)?")
            .arg(page->listDisplayString())
            .arg(measurementCount);
    } else {
        message = QString("Delete page '%1'?").arg(page->listDisplayString());
    }
    
    QMessageBox::StandardButton ret = QMessageBox::question(
        this, "Delete Page", message,
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
    if (ret != QMessageBox::Yes) {
        return;
    }
    
    // Remove from UI
    m_pagesPanel->removePage(pageId);
    
    // Clear view if this was the current page
    if (m_currentPageId == pageId) {
        m_blueprintView->clearImage();
        m_measurementPanel->clearMeasurements();
        m_currentPageId.clear();
        m_deletePageAction->setEnabled(false);
    }
    
    // Remove from project (this also removes measurements)
    m_project.removePage(pageId);
    
    setDirty(true);
    updateQuoteSummary();
    
    // Select another page if available
    if (m_project.pages().size() > 0) {
        m_pagesPanel->selectPage(m_project.pages().first().id());
    } else {
        m_deletePageAction->setEnabled(false);
    }
    
    updateStatusBar("Page deleted.");
}

void MainWindow::onUndoStackCleanChanged(bool clean)
{
    // When undo stack becomes clean, we're at the saved state
    // When it becomes dirty (clean=false), we have unsaved changes
    // Note: This doesn't cover all dirty cases (like calibration changes)
    if (!clean) {
        setDirty(true);
    }
}

// ============================================================================
// Properties Dock Slots
// ============================================================================

void MainWindow::onPropertyNameChanged(int id, const QString& oldVal, const QString& newVal)
{
    Measurement* m = m_project.findMeasurement(id);
    if (m) {
        m->setName(newVal);
        m_measurementPanel->updateMeasurement(*m);
        setDirty(true);
        m_undoStack->push(new SetMeasurementFieldCommand(
            this, id, MeasurementField::Name, oldVal, newVal));
    }
}

void MainWindow::onPropertyNotesChanged(int id, const QString& oldVal, const QString& newVal)
{
    Measurement* m = m_project.findMeasurement(id);
    if (m) {
        m->setNotes(newVal);
        setDirty(true);
        m_undoStack->push(new SetMeasurementFieldCommand(
            this, id, MeasurementField::Notes, oldVal, newVal));
    }
}

void MainWindow::onPropertyCategoryChanged(int id, Category oldVal, Category newVal)
{
    Measurement* m = m_project.findMeasurement(id);
    if (m) {
        m->setCategory(newVal);
        m_measurementPanel->updateMeasurement(*m);
        setDirty(true);
        updateQuoteSummary();
        m_undoStack->push(new SetMeasurementFieldCommand(
            this, id, MeasurementField::Category,
            static_cast<int>(oldVal), static_cast<int>(newVal)));
    }
}

void MainWindow::onPropertyMaterialTypeChanged(int id, MaterialType oldVal, MaterialType newVal)
{
    Measurement* m = m_project.findMeasurement(id);
    if (m) {
        m->setMaterialType(newVal);
        m_measurementPanel->updateMeasurement(*m);
        setDirty(true);
        updateQuoteSummary();
        m_undoStack->push(new SetMeasurementFieldCommand(
            this, id, MeasurementField::MaterialType,
            static_cast<int>(oldVal), static_cast<int>(newVal)));
    }
}

void MainWindow::onPropertySizeChanged(int id, const QString& oldVal, const QString& newVal)
{
    Measurement* m = m_project.findMeasurement(id);
    if (m) {
        m->setSize(newVal);
        m_measurementPanel->updateMeasurement(*m);
        setDirty(true);
        updateQuoteSummary();
        m_undoStack->push(new SetMeasurementFieldCommand(
            this, id, MeasurementField::Size, oldVal, newVal));
    }
}

void MainWindow::onPropertyLaborClassChanged(int id, LaborClass oldVal, LaborClass newVal)
{
    Measurement* m = m_project.findMeasurement(id);
    if (m) {
        m->setLaborClass(newVal);
        m_measurementPanel->updateMeasurement(*m);
        setDirty(true);
        updateQuoteSummary();
        m_undoStack->push(new SetMeasurementFieldCommand(
            this, id, MeasurementField::LaborClass,
            static_cast<int>(oldVal), static_cast<int>(newVal)));
    }
}

void MainWindow::onPickShapeRequested(int measurementId)
{
    Measurement* m = m_project.findMeasurement(measurementId);
    if (!m) {
        return;
    }

    // Check if database has shapes
    if (!m_shapesDb.isOpen() || !m_shapesDb.hasShapes()) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            this, "No Shapes Imported",
            "No AISC shapes have been imported yet.\n\n"
            "Would you like to import shapes from an AISC spreadsheet now?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            onImportAiscShapes();
        }
        
        if (!m_shapesDb.hasShapes()) {
            return;
        }
    }

    // Open shape picker dialog
    ShapePickerDialog dialog(&m_shapesDb, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    int newShapeId = dialog.selectedShapeId();
    QString newShapeLabel = dialog.selectedShapeLabel();
    
    if (newShapeId < 0) {
        return;
    }

    // Store old values for undo
    int oldShapeId = m->shapeId();
    QString oldShapeLabel = m->shapeLabel();
    QString oldSize = m->size();

    // Update measurement
    m->setShapeId(newShapeId);
    m->setShapeLabel(newShapeLabel);
    m->setSize(newShapeLabel);  // Auto-fill size with shape label

    // Update UI
    m_measurementPanel->updateMeasurement(*m);
    m_propertiesDock->updateFromMeasurement(m);
    
    setDirty(true);
    updateQuoteSummary();

    // Push undo command for size change (shape changes are part of this)
    m_undoStack->push(new SetMeasurementFieldCommand(
        this, measurementId, MeasurementField::Size, oldSize, newShapeLabel));
}

// ============================================================================
// Quote Dock Slots
// ============================================================================

void MainWindow::onQuoteRatesChanged(const QuoteRates& rates)
{
    m_project.setQuoteRates(rates);
    setDirty(true);
}

void MainWindow::onCurrentPageOnlyChanged(bool currentPageOnly)
{
    Q_UNUSED(currentPageOnly);
    updateQuoteSummary();
}

// ============================================================================
// Internal Methods
// ============================================================================

void MainWindow::addMeasurementInternal(const Measurement& measurement)
{
    m_project.addMeasurement(measurement);
    
    // Only add to panel if it's for the current page
    if (measurement.pageId() == m_currentPageId) {
        m_measurementPanel->addMeasurement(measurement);
        m_blueprintView->addMeasurement(measurement);
    }
    
    setDirty(true);
    updateQuoteSummary();
}

void MainWindow::removeMeasurementInternal(int measurementId)
{
    // Check if this measurement is on the current page
    const Measurement* m = m_project.findMeasurement(measurementId);
    bool isCurrentPage = (m && m->pageId() == m_currentPageId);
    
    m_project.removeMeasurement(measurementId);
    
    if (isCurrentPage) {
        m_measurementPanel->removeMeasurement(measurementId);
        m_blueprintView->removeMeasurement(measurementId);
    }
    
    m_deleteAction->setEnabled(false);
    
    if (m_selectedMeasurementId == measurementId) {
        m_selectedMeasurementId = -1;
        m_propertiesDock->clearSelection();
    }
    
    setDirty(true);
    updateQuoteSummary();
}

void MainWindow::setMeasurementFieldInternal(int measurementId, MeasurementField field, const QVariant& value)
{
    Measurement* m = m_project.findMeasurement(measurementId);
    if (!m) return;

    switch (field) {
        case MeasurementField::Name:
            m->setName(value.toString());
            if (m->pageId() == m_currentPageId) {
                m_measurementPanel->updateMeasurement(*m);
            }
            break;
        case MeasurementField::Notes:
            m->setNotes(value.toString());
            break;
        case MeasurementField::Category:
            m->setCategory(static_cast<Category>(value.toInt()));
            break;
        case MeasurementField::MaterialType:
            m->setMaterialType(static_cast<MaterialType>(value.toInt()));
            break;
        case MeasurementField::Size:
            m->setSize(value.toString());
            if (m->pageId() == m_currentPageId) {
                m_measurementPanel->updateMeasurement(*m);
            }
            break;
        case MeasurementField::LaborClass:
            m->setLaborClass(static_cast<LaborClass>(value.toInt()));
            break;
        case MeasurementField::ShapeId:
            m->setShapeId(value.toInt());
            break;
        case MeasurementField::ShapeLabel:
            m->setShapeLabel(value.toString());
            break;
    }

    // Update properties panel if this is the selected measurement
    if (m_selectedMeasurementId == measurementId) {
        m_propertiesDock->updateFromMeasurement(m);
    }

    setDirty(true);
    updateQuoteSummary();
}

void MainWindow::updateStatusBar(const QString& message)
{
    statusBar()->showMessage(message);
}

void MainWindow::updateWindowTitle()
{
    QString title = "Blueprint Takeoff";
    
    if (!m_currentProjectPath.isEmpty()) {
        QFileInfo fi(m_currentProjectPath);
        title = QString("%1 - %2").arg(fi.fileName(), title);
    }
    
    if (m_dirty) {
        title = "* " + title;
    }
    
    setWindowTitle(title);
}

void MainWindow::setDirty(bool dirty)
{
    if (m_dirty != dirty) {
        m_dirty = dirty;
        updateWindowTitle();
    }
}

bool MainWindow::maybeSave()
{
    if (!m_dirty) {
        return true;
    }

    QMessageBox::StandardButton ret = QMessageBox::warning(
        this,
        "Unsaved Changes",
        "The project has been modified.\nDo you want to save your changes?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );

    switch (ret) {
        case QMessageBox::Save:
            onSaveProject();
            return !m_dirty;  // Returns true if save succeeded
        case QMessageBox::Discard:
            return true;
        case QMessageBox::Cancel:
        default:
            return false;
    }
}

bool MainWindow::saveProject(const QString& filePath)
{
    // Sync current page's calibration
    if (!m_currentPageId.isEmpty()) {
        Page* page = m_project.findPage(m_currentPageId);
        if (page) {
            page->calibration() = m_blueprintView->calibration();
        }
    }
    
    // Save quote rates from dock
    m_project.setQuoteRates(m_quoteDock->currentRates());
    
    if (m_project.saveToJson(filePath)) {
        m_currentProjectPath = filePath;
        setDirty(false);
        m_undoStack->setClean();
        return true;
    } else {
        QMessageBox::critical(this, "Save Error", m_project.lastError());
        return false;
    }
}

bool MainWindow::loadProject(const QString& filePath)
{
    Project newProject;
    if (!newProject.loadFromJson(filePath)) {
        QMessageBox::critical(this, "Load Error", newProject.lastError());
        return false;
    }

    // Clear current state
    clearProject();
    
    // Set new project
    m_project = newProject;
    m_currentProjectPath = filePath;
    
    // Populate pages panel
    for (const Page& page : m_project.pages()) {
        m_pagesPanel->addPage(page);
    }
    
    // Select first page if available
    if (!m_project.pages().isEmpty()) {
        m_pagesPanel->selectPage(m_project.pages().first().id());
    }
    
    // Load quote rates
    m_quoteDock->setRates(m_project.quoteRates());
    updateQuoteSummary();
    
    setDirty(false);
    updateWindowTitle();
    
    return true;
}

void MainWindow::clearProject()
{
    m_project.clear();
    m_currentProjectPath.clear();
    m_currentPageId.clear();
    m_undoStack->clear();
    m_pagesPanel->clearPages();
    m_blueprintView->clearImage();
    m_measurementPanel->clearMeasurements();
    m_propertiesDock->clearSelection();
    m_quoteDock->setRates(QuoteRates());
    m_selectedMeasurementId = -1;
    m_deleteAction->setEnabled(false);
    m_deletePageAction->setEnabled(false);
    setDirty(false);
    updateWindowTitle();
    updateQuoteSummary();
    
    // Reset to pan mode
    m_noneToolAction->setChecked(true);
    m_blueprintView->setTool(Tool::None);
}

void MainWindow::syncProjectFromView()
{
    // Sync current page's calibration
    if (!m_currentPageId.isEmpty()) {
        Page* page = m_project.findPage(m_currentPageId);
        if (page) {
            page->calibration() = m_blueprintView->calibration();
        }
    }
}

void MainWindow::syncViewFromProject()
{
    // This is now handled by loadCurrentPage() and updateMeasurementPanelForPage()
}

void MainWindow::updateQuoteSummary()
{
    bool currentPageOnly = m_quoteDock->isCurrentPageOnly();
    
    if (currentPageOnly && !m_currentPageId.isEmpty()) {
        // Filter to current page
        QVector<Measurement> pageMeasurements = m_project.measurementsForPage(m_currentPageId);
        m_quoteDock->updateFromMeasurements(pageMeasurements, m_project.quoteRates(), &m_shapesDb);
    } else {
        // All measurements
        m_quoteDock->updateFromMeasurements(m_project.measurements(), m_project.quoteRates(), &m_shapesDb);
    }
}

void MainWindow::updatePropertiesPanel()
{
    if (m_selectedMeasurementId >= 0) {
        const Measurement* m = m_project.findMeasurement(m_selectedMeasurementId);
        m_propertiesDock->setMeasurement(m, m_selectedMeasurementId);
    } else {
        m_propertiesDock->clearSelection();
    }
}

void MainWindow::loadCurrentPage()
{
    if (m_currentPageId.isEmpty()) {
        m_blueprintView->clearImage();
        return;
    }
    
    Page* page = m_project.findPage(m_currentPageId);
    if (!page) {
        m_blueprintView->clearImage();
        return;
    }
    
    bool loaded = false;
    
    if (page->type() == PageType::Image) {
        // Load image directly
        loaded = m_blueprintView->loadImage(page->sourcePath());
        if (!loaded) {
            QMessageBox::warning(this, "Image Not Found",
                QString("Could not load image: %1\nThe image file may have been moved or deleted.")
                .arg(page->sourcePath()));
        }
    } else if (page->type() == PageType::Pdf) {
        // Render PDF page
        if (!m_pdfRenderer.isOpen() || m_pdfRenderer.currentPath() != page->sourcePath()) {
            if (!m_pdfRenderer.openPdf(page->sourcePath())) {
                QMessageBox::warning(this, "PDF Not Found",
                    QString("Could not open PDF: %1\n%2")
                    .arg(page->sourcePath(), m_pdfRenderer.lastError()));
                return;
            }
        }
        
        QImage image = m_pdfRenderer.renderPage(page->pdfPageIndex());
        if (image.isNull()) {
            QMessageBox::warning(this, "Render Error",
                QString("Could not render PDF page: %1").arg(m_pdfRenderer.lastError()));
        } else {
            loaded = m_blueprintView->loadFromImage(image);
        }
    }
    
    if (loaded) {
        // Restore calibration for this page
        m_blueprintView->setCalibration(page->calibration());
        
        // Restore measurements for this page
        int maxId = 0;
        QVector<Measurement> pageMeasurements = m_project.measurementsForPage(m_currentPageId);
        for (const Measurement& m : pageMeasurements) {
            m_blueprintView->addMeasurement(m);
            if (m.id() > maxId) {
                maxId = m.id();
            }
        }
        
        // Find the max ID across all measurements (not just this page)
        for (const Measurement& m : m_project.measurements()) {
            if (m.id() > maxId) {
                maxId = m.id();
            }
        }
        m_blueprintView->setNextMeasurementId(maxId + 1);
    }
}

void MainWindow::updateMeasurementPanelForPage()
{
    m_measurementPanel->clearMeasurements();
    
    if (m_currentPageId.isEmpty()) {
        return;
    }
    
    QVector<Measurement> pageMeasurements = m_project.measurementsForPage(m_currentPageId);
    for (const Measurement& m : pageMeasurements) {
        m_measurementPanel->addMeasurement(m);
    }
}
