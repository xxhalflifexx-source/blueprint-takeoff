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
    , m_itemsPanel(nullptr)
    , m_propertiesDock(nullptr)
    , m_quoteDock(nullptr)
    , m_mainSplitter(nullptr)
    , m_leftSplitter(nullptr)
    , m_toolBar(nullptr)
    , m_newProjectAction(nullptr)
    , m_openProjectAction(nullptr)
    , m_addImagePageAction(nullptr)
    , m_addPdfAction(nullptr)
    , m_importShapesAction(nullptr)
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
    , m_currentPageId()
    , m_selectedItemId(-1)
{
    m_undoStack = new QUndoStack(this);
    
    setupUi();
    connectSignals();
    updateWindowTitle();
    resize(1400, 900);
}

MainWindow::~MainWindow()
{
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

    m_newProjectAction = new QAction("&New Project...", this);
    m_newProjectAction->setShortcut(QKeySequence::New);
    m_newProjectAction->setStatusTip("Create a new project");
    fileMenu->addAction(m_newProjectAction);

    m_openProjectAction = new QAction("&Open Project...", this);
    m_openProjectAction->setShortcut(QKeySequence::Open);
    m_openProjectAction->setStatusTip("Open an existing project file");
    fileMenu->addAction(m_openProjectAction);

    fileMenu->addSeparator();

    m_addImagePageAction = new QAction("Add &Image Page...", this);
    m_addImagePageAction->setStatusTip("Add an image (PNG/JPG) as a new page");
    m_addImagePageAction->setEnabled(false);
    fileMenu->addAction(m_addImagePageAction);

    m_addPdfAction = new QAction("Add P&DF...", this);
    m_addPdfAction->setStatusTip("Add pages from a PDF file");
    m_addPdfAction->setEnabled(false);
    fileMenu->addAction(m_addPdfAction);

    fileMenu->addSeparator();

    m_importShapesAction = new QAction("Import AISC &Shapes...", this);
    m_importShapesAction->setStatusTip("Import AISC shapes from a CSV file");
    m_importShapesAction->setEnabled(false);
    fileMenu->addAction(m_importShapesAction);

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

    m_deleteAction = new QAction("&Delete Item", this);
    m_deleteAction->setShortcut(QKeySequence::Delete);
    m_deleteAction->setStatusTip("Delete the selected takeoff item");
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

    // Add open button to toolbar
    m_toolBar->addAction(m_openProjectAction);
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

    // Left panel: Pages + Items in vertical splitter
    m_leftSplitter = new QSplitter(Qt::Vertical, m_mainSplitter);
    
    // Pages panel
    m_pagesPanel = new PagesPanel(m_leftSplitter);
    m_leftSplitter->addWidget(m_pagesPanel);
    
    // Items panel (was Measurements panel)
    m_itemsPanel = new MeasurementPanel(m_leftSplitter);
    m_leftSplitter->addWidget(m_itemsPanel);
    
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
    connect(m_addImagePageAction, &QAction::triggered, this, &MainWindow::onAddImagePage);
    connect(m_addPdfAction, &QAction::triggered, this, &MainWindow::onAddPdf);
    connect(m_importShapesAction, &QAction::triggered, this, &MainWindow::onImportShapes);
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExit);

    // Edit menu actions
    connect(m_deleteAction, &QAction::triggered, this, &MainWindow::onDeleteItem);
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

    // Items panel signals
    connect(m_itemsPanel, &MeasurementPanel::measurementSelected,
            this, &MainWindow::onItemSelected);

    // Undo stack signals
    connect(m_undoStack, &QUndoStack::cleanChanged,
            this, &MainWindow::onUndoStackCleanChanged);

    // Properties dock signals
    connect(m_propertiesDock, &PropertiesDock::designationChanged,
            this, &MainWindow::onDesignationChanged);
    connect(m_propertiesDock, &PropertiesDock::qtyChanged,
            this, &MainWindow::onQtyChanged);
    connect(m_propertiesDock, &PropertiesDock::notesChanged,
            this, &MainWindow::onNotesChanged);
    connect(m_propertiesDock, &PropertiesDock::pickShapeRequested,
            this, &MainWindow::onPickShapeRequested);

    // Quote dock signals
    connect(m_quoteDock, &QuoteDock::materialPriceChanged,
            this, &MainWindow::onMaterialPriceChanged);
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

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Create New Project",
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

    clearProject();

    if (m_project.create(filePath)) {
        // Enable project-specific actions
        m_addImagePageAction->setEnabled(true);
        m_addPdfAction->setEnabled(true);
        m_importShapesAction->setEnabled(true);
        
        // Load material price from project
        m_quoteDock->setMaterialPricePerLb(m_project.materialPricePerLb());
        
        updateWindowTitle();
        updateStatusBar(QString("New project created: %1. Add an image or PDF to begin.")
                       .arg(QFileInfo(filePath).fileName()));
    } else {
        QMessageBox::critical(this, "Error", 
            QString("Failed to create project: %1").arg(m_project.lastError()));
    }
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

    clearProject();

    if (m_project.open(filePath)) {
        // Enable project-specific actions
        m_addImagePageAction->setEnabled(true);
        m_addPdfAction->setEnabled(true);
        m_importShapesAction->setEnabled(true);
        
        // Populate pages panel
        for (const Page& page : m_project.pages()) {
            m_pagesPanel->addPage(page);
        }
        
        // Select first page if available
        if (!m_project.pages().isEmpty()) {
            m_pagesPanel->selectPage(m_project.pages().first().id());
        }
        
        // Load settings
        m_quoteDock->setMaterialPricePerLb(m_project.materialPricePerLb());
        refreshDesignationAutocomplete();
        updateQuoteSummary();
        
        updateWindowTitle();
        updateStatusBar(QString("Project loaded: %1").arg(QFileInfo(filePath).fileName()));
    } else {
        QMessageBox::critical(this, "Error", 
            QString("Failed to open project: %1").arg(m_project.lastError()));
    }
}

void MainWindow::onAddImagePage()
{
    if (!m_project.isOpen()) {
        return;
    }

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
    
    // Select the new page
    m_pagesPanel->selectPage(page.id());
    
    updateStatusBar(QString("Added page: %1. Calibrate before measuring.").arg(page.listDisplayString()));
}

void MainWindow::onAddPdf()
{
    if (!m_project.isOpen()) {
        return;
    }

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

    // Select the first imported page
    if (!firstPageId.isEmpty()) {
        m_pagesPanel->selectPage(firstPageId);
    }

    int count = toPage - fromPage + 1;
    updateStatusBar(QString("Added %1 page(s) from PDF. Calibrate before measuring.").arg(count));
}

void MainWindow::onImportShapes()
{
    if (!m_project.isOpen()) {
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Import AISC Shapes",
        QString(),
        "CSV Files (*.csv);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    int count = m_project.importShapesFromCsv(filePath);

    if (count < 0) {
        QMessageBox::critical(this, "Import Error",
            QString("Failed to import shapes: %1").arg(m_project.lastError()));
    } else if (count == 0) {
        QMessageBox::warning(this, "Import", "No shapes were imported from the file.");
    } else {
        refreshDesignationAutocomplete();
        QMessageBox::information(this, "Import Complete",
            QString("Successfully imported %1 shapes.\n\nTotal shapes in project: %2")
            .arg(count).arg(m_project.shapeCount()));
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

void MainWindow::onDeleteItem()
{
    int selectedId = m_itemsPanel->selectedMeasurementId();
    if (selectedId < 0) {
        return;
    }

    // Find the item
    TakeoffItem* item = m_project.findTakeoffItem(selectedId);
    if (!item) {
        return;
    }

    // Create copy for undo
    TakeoffItem copy = *item;
    
    // Remove the item
    removeTakeoffItemInternal(selectedId);
    
    // Push undo command
    m_undoStack->push(new DeleteTakeoffItemCommand(this, copy));
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
        m_project.updatePage(*page);
    }
    
    updateStatusBar(QString("Calibration complete: %1 pixels/inch. Ready to measure.")
                    .arg(pixelsPerInch, 0, 'f', 2));
    
    // Switch to pan mode after calibration
    m_noneToolAction->setChecked(true);
}

void MainWindow::onMeasurementCompleted(const Measurement& measurement)
{
    // Convert Measurement to TakeoffItem
    TakeoffItem item;
    item.setPageId(m_currentPageId);
    item.setKind(measurement.type() == MeasurementType::Line ? 
                 TakeoffItem::Line : TakeoffItem::Polyline);
    item.setPoints(measurement.points());
    item.setLengthInches(measurement.lengthInches());
    item.setQty(1);
    
    // Add to project (gets ID assigned)
    addTakeoffItemInternal(item);
    
    // Push undo command
    m_undoStack->push(new AddTakeoffItemCommand(this, item));
    
    // Auto-select the new item and focus designation field
    m_itemsPanel->selectMeasurement(item.id());
    m_propertiesDock->focusDesignationField();
    
    updateStatusBar(QString("Item added: %1 - assign material.").arg(item.displayString()));
}

void MainWindow::onLiveMeasurementChanged(double inches)
{
    if (inches > 0.0) {
        updateStatusBar(QString("Current: %1 in").arg(inches, 0, 'f', 2));
    }
}

void MainWindow::onItemSelected(int itemId)
{
    m_selectedItemId = itemId;
    m_blueprintView->highlightMeasurement(itemId);
    m_deleteAction->setEnabled(itemId >= 0);
    
    // Update properties panel
    updatePropertiesPanel();
    
    if (itemId >= 0) {
        const TakeoffItem* item = m_project.findTakeoffItem(itemId);
        if (item) {
            updateStatusBar(QString("Selected: %1").arg(item->displayString()));
        }
    }
}

void MainWindow::onToolCancelled()
{
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
            m_project.updatePage(*prevPage);
        }
    }
    
    m_currentPageId = pageId;
    m_selectedItemId = -1;
    
    // Load the new page
    loadCurrentPage();
    
    // Update items panel for this page
    updateItemsPanelForPage();
    
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
    int itemCount = m_project.takeoffItemsForPage(pageId).size();
    QString message;
    if (itemCount > 0) {
        message = QString("Delete page '%1' and its %2 item(s)?")
            .arg(page->listDisplayString())
            .arg(itemCount);
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
        m_itemsPanel->clearMeasurements();
        m_currentPageId.clear();
        m_deletePageAction->setEnabled(false);
    }
    
    // Remove from project
    m_project.removePage(pageId);
    
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
    Q_UNUSED(clean);
    // Project auto-saves with SQLite, so we don't track dirty state the same way
}

// ============================================================================
// Properties Dock Slots
// ============================================================================

void MainWindow::onDesignationChanged(int itemId, const QString& oldVal, const QString& newVal,
                                       int oldShapeId, int newShapeId)
{
    TakeoffItem* item = m_project.findTakeoffItem(itemId);
    if (!item) return;

    // Look up shape by designation
    auto shape = m_project.getShapeByDesignation(newVal);
    int resolvedShapeId = shape.id;
    
    item->setDesignation(newVal);
    item->setShapeId(resolvedShapeId);
    m_project.updateTakeoffItem(*item);
    
    // Update UI
    updateItemDisplay(itemId);
    updatePropertiesPanel();
    updateQuoteSummary();
    
    // Push undo command
    m_undoStack->push(new SetTakeoffItemFieldCommand(
        this, itemId, TakeoffItemField::Designation, oldVal, newVal));
}

void MainWindow::onQtyChanged(int itemId, int oldVal, int newVal)
{
    TakeoffItem* item = m_project.findTakeoffItem(itemId);
    if (!item) return;

    item->setQty(newVal);
    m_project.updateTakeoffItem(*item);
    
    updateItemDisplay(itemId);
    updatePropertiesPanel();
    updateQuoteSummary();
    
    m_undoStack->push(new SetTakeoffItemFieldCommand(
        this, itemId, TakeoffItemField::Qty, oldVal, newVal));
}

void MainWindow::onNotesChanged(int itemId, const QString& oldVal, const QString& newVal)
{
    TakeoffItem* item = m_project.findTakeoffItem(itemId);
    if (!item) return;

    item->setNotes(newVal);
    m_project.updateTakeoffItem(*item);
    
    m_undoStack->push(new SetTakeoffItemFieldCommand(
        this, itemId, TakeoffItemField::Notes, oldVal, newVal));
}

void MainWindow::onPickShapeRequested(int itemId)
{
    TakeoffItem* item = m_project.findTakeoffItem(itemId);
    if (!item) return;

    // Check if shapes are imported
    if (!m_project.hasShapes()) {
        QMessageBox::StandardButton ret = QMessageBox::question(
            this, "No Shapes Imported",
            "No AISC shapes have been imported yet.\n\n"
            "Would you like to import shapes from a CSV file now?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            onImportShapes();
        }
        
        if (!m_project.hasShapes()) {
            return;
        }
    }

    // Open shape picker dialog
    ShapePickerDialog dialog(m_project.database(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    int newShapeId = dialog.selectedShapeId();
    QString newDesignation = dialog.selectedShapeLabel();
    
    if (newShapeId < 0) {
        return;
    }

    // Store old values for undo
    QString oldDesignation = item->designation();

    // Update item
    item->setDesignation(newDesignation);
    item->setShapeId(newShapeId);
    m_project.updateTakeoffItem(*item);
    
    // Update UI
    updateItemDisplay(itemId);
    updatePropertiesPanel();
    updateQuoteSummary();

    m_undoStack->push(new SetTakeoffItemFieldCommand(
        this, itemId, TakeoffItemField::Designation, oldDesignation, newDesignation));
}

// ============================================================================
// Quote Dock Slots
// ============================================================================

void MainWindow::onMaterialPriceChanged(double pricePerLb)
{
    if (m_project.isOpen()) {
        m_project.setMaterialPricePerLb(pricePerLb);
        updatePropertiesPanel();
    }
}

void MainWindow::onCurrentPageOnlyChanged(bool currentPageOnly)
{
    Q_UNUSED(currentPageOnly);
    updateQuoteSummary();
}

// ============================================================================
// Internal Methods
// ============================================================================

void MainWindow::addTakeoffItemInternal(TakeoffItem& item)
{
    int newId = m_project.addTakeoffItem(item);
    
    // Only add to panel if it's for the current page
    if (item.pageId() == m_currentPageId) {
        // Convert to Measurement for display (temporary compatibility)
        Measurement m;
        m.setId(newId);
        m.setPageId(item.pageId());
        m.setType(item.kind() == TakeoffItem::Line ? MeasurementType::Line : MeasurementType::Polyline);
        m.setPoints(item.points());
        m.setLengthInches(item.lengthInches());
        m.setSize(item.designation());
        
        m_itemsPanel->addMeasurement(m);
        m_blueprintView->addMeasurement(m);
    }
    
    updateQuoteSummary();
}

void MainWindow::removeTakeoffItemInternal(int itemId)
{
    const TakeoffItem* item = m_project.findTakeoffItem(itemId);
    bool isCurrentPage = (item && item->pageId() == m_currentPageId);
    
    m_project.removeTakeoffItem(itemId);
    
    if (isCurrentPage) {
        m_itemsPanel->removeMeasurement(itemId);
        m_blueprintView->removeMeasurement(itemId);
    }
    
    m_deleteAction->setEnabled(false);
    
    if (m_selectedItemId == itemId) {
        m_selectedItemId = -1;
        m_propertiesDock->clearSelection();
    }
    
    updateQuoteSummary();
}

void MainWindow::setTakeoffItemFieldInternal(int itemId, TakeoffItemField field, const QVariant& value)
{
    TakeoffItem* item = m_project.findTakeoffItem(itemId);
    if (!item) return;

    switch (field) {
        case TakeoffItemField::Designation: {
            QString designation = value.toString();
            item->setDesignation(designation);
            // Also update shape ID
            auto shape = m_project.getShapeByDesignation(designation);
            item->setShapeId(shape.id);
            break;
        }
        case TakeoffItemField::Qty:
            item->setQty(value.toInt());
            break;
        case TakeoffItemField::Notes:
            item->setNotes(value.toString());
            break;
        case TakeoffItemField::ShapeId:
            item->setShapeId(value.toInt());
            break;
    }

    m_project.updateTakeoffItem(*item);

    // Update properties panel if this is the selected item
    if (m_selectedItemId == itemId) {
        m_propertiesDock->updateFromItem(item);
    }

    updateQuoteSummary();
}

void MainWindow::updateStatusBar(const QString& message)
{
    statusBar()->showMessage(message);
}

void MainWindow::updateWindowTitle()
{
    QString title = "Blueprint Takeoff";
    
    if (m_project.isOpen()) {
        QFileInfo fi(m_project.filePath());
        title = QString("%1 - %2").arg(fi.fileName(), title);
    }
    
    setWindowTitle(title);
}

bool MainWindow::maybeSave()
{
    // With SQLite, changes are auto-saved, so just return true
    return true;
}

void MainWindow::clearProject()
{
    m_project.close();
    m_currentPageId.clear();
    m_undoStack->clear();
    m_pagesPanel->clearPages();
    m_blueprintView->clearImage();
    m_itemsPanel->clearMeasurements();
    m_propertiesDock->clearSelection();
    m_propertiesDock->setDesignationList(QStringList());
    m_selectedItemId = -1;
    m_deleteAction->setEnabled(false);
    m_deletePageAction->setEnabled(false);
    m_addImagePageAction->setEnabled(false);
    m_addPdfAction->setEnabled(false);
    m_importShapesAction->setEnabled(false);
    updateWindowTitle();
    updateQuoteSummary();
    
    // Reset to pan mode
    m_noneToolAction->setChecked(true);
    m_blueprintView->setTool(Tool::None);
}

void MainWindow::updateQuoteSummary()
{
    m_quoteDock->updateFromProject(&m_project);
}

void MainWindow::updatePropertiesPanel()
{
    if (m_selectedItemId >= 0) {
        const TakeoffItem* item = m_project.findTakeoffItem(m_selectedItemId);
        if (item) {
            m_propertiesDock->setTakeoffItem(item, m_selectedItemId);
            
            // Update computed values
            double wLbPerFt = 0.0;
            if (item->shapeId() > 0) {
                auto shape = m_project.getShape(item->shapeId());
                wLbPerFt = shape.wLbPerFt;
            }
            m_propertiesDock->updateComputedValues(wLbPerFt, m_project.materialPricePerLb());
        } else {
            m_propertiesDock->clearSelection();
        }
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
    
    if (page->type() == Page::Image) {
        loaded = m_blueprintView->loadImage(page->sourcePath());
        if (!loaded) {
            QMessageBox::warning(this, "Image Not Found",
                QString("Could not load image: %1\nThe image file may have been moved or deleted.")
                .arg(page->sourcePath()));
        }
    } else if (page->type() == Page::Pdf) {
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
        
        // Restore items for this page (as Measurement for display)
        int maxId = 0;
        QVector<TakeoffItem> pageItems = m_project.takeoffItemsForPage(m_currentPageId);
        for (const TakeoffItem& item : pageItems) {
            Measurement m;
            m.setId(item.id());
            m.setPageId(item.pageId());
            m.setType(item.kind() == TakeoffItem::Line ? MeasurementType::Line : MeasurementType::Polyline);
            m.setPoints(item.points());
            m.setLengthInches(item.lengthInches());
            m.setSize(item.designation());
            
            m_blueprintView->addMeasurement(m);
            if (item.id() > maxId) {
                maxId = item.id();
            }
        }
        
        // Find the max ID across all items
        for (const TakeoffItem& item : m_project.takeoffItems()) {
            if (item.id() > maxId) {
                maxId = item.id();
            }
        }
        m_blueprintView->setNextMeasurementId(maxId + 1);
    }
}

void MainWindow::updateItemsPanelForPage()
{
    m_itemsPanel->clearMeasurements();
    
    if (m_currentPageId.isEmpty()) {
        return;
    }
    
    QVector<TakeoffItem> pageItems = m_project.takeoffItemsForPage(m_currentPageId);
    for (const TakeoffItem& item : pageItems) {
        // Convert to Measurement for display
        Measurement m;
        m.setId(item.id());
        m.setPageId(item.pageId());
        m.setType(item.kind() == TakeoffItem::Line ? MeasurementType::Line : MeasurementType::Polyline);
        m.setPoints(item.points());
        m.setLengthInches(item.lengthInches());
        m.setSize(item.designation());
        
        m_itemsPanel->addMeasurement(m);
    }
}

void MainWindow::refreshDesignationAutocomplete()
{
    if (m_project.isOpen()) {
        m_propertiesDock->setDesignationList(m_project.allDesignations());
    }
}

// Helper function for updating item display
void MainWindow::updateItemDisplay(int itemId)
{
    const TakeoffItem* item = m_project.findTakeoffItem(itemId);
    if (!item || item->pageId() != m_currentPageId) {
        return;
    }
    
    // Convert to Measurement for display
    Measurement m;
    m.setId(item->id());
    m.setPageId(item->pageId());
    m.setType(item->kind() == TakeoffItem::Line ? MeasurementType::Line : MeasurementType::Polyline);
    m.setPoints(item->points());
    m.setLengthInches(item->lengthInches());
    m.setSize(item->designation());
    
    m_itemsPanel->updateMeasurement(m);
}
