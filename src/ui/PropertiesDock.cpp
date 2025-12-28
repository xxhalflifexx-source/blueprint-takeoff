#include "PropertiesDock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

PropertiesDock::PropertiesDock(QWidget* parent)
    : QDockWidget("Properties", parent)
    , m_container(nullptr)
    , m_formLayout(nullptr)
    , m_noSelectionLabel(nullptr)
    , m_nameEdit(nullptr)
    , m_notesEdit(nullptr)
    , m_categoryCombo(nullptr)
    , m_materialTypeCombo(nullptr)
    , m_sizeEdit(nullptr)
    , m_pickShapeButton(nullptr)
    , m_laborClassCombo(nullptr)
    , m_infoLabel(nullptr)
    , m_currentMeasurementId(-1)
    , m_updatingFields(false)
{
    setupUi();
    clearSelection();
}

PropertiesDock::~PropertiesDock()
{
}

void PropertiesDock::setupUi()
{
    // Main container
    m_container = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_container);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // "No selection" label
    m_noSelectionLabel = new QLabel("No measurement selected", m_container);
    m_noSelectionLabel->setAlignment(Qt::AlignCenter);
    m_noSelectionLabel->setStyleSheet("color: gray; font-style: italic;");
    mainLayout->addWidget(m_noSelectionLabel);

    // Form layout for fields
    m_formLayout = new QFormLayout();
    m_formLayout->setLabelAlignment(Qt::AlignRight);
    mainLayout->addLayout(m_formLayout);

    // Info label
    m_infoLabel = new QLabel(m_container);
    m_infoLabel->setStyleSheet("font-weight: bold; margin-bottom: 10px;");
    m_formLayout->addRow(m_infoLabel);

    // Name
    m_nameEdit = new QLineEdit(m_container);
    m_nameEdit->setPlaceholderText("Enter name...");
    m_formLayout->addRow("Name:", m_nameEdit);
    connect(m_nameEdit, &QLineEdit::editingFinished, this, &PropertiesDock::onNameEditingFinished);

    // Category
    m_categoryCombo = new QComboBox(m_container);
    m_formLayout->addRow("Category:", m_categoryCombo);
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PropertiesDock::onCategoryChanged);

    // Material Type
    m_materialTypeCombo = new QComboBox(m_container);
    m_formLayout->addRow("Material:", m_materialTypeCombo);
    connect(m_materialTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PropertiesDock::onMaterialTypeChanged);

    // Size with Pick button
    QWidget* sizeWidget = new QWidget(m_container);
    QHBoxLayout* sizeLayout = new QHBoxLayout(sizeWidget);
    sizeLayout->setContentsMargins(0, 0, 0, 0);
    sizeLayout->setSpacing(4);
    
    m_sizeEdit = new QLineEdit(sizeWidget);
    m_sizeEdit->setPlaceholderText("e.g. W14X90");
    sizeLayout->addWidget(m_sizeEdit, 1);
    
    m_pickShapeButton = new QPushButton("Pick...", sizeWidget);
    m_pickShapeButton->setToolTip("Pick from AISC shapes database");
    m_pickShapeButton->setMaximumWidth(50);
    sizeLayout->addWidget(m_pickShapeButton);
    
    m_formLayout->addRow("Size:", sizeWidget);
    connect(m_sizeEdit, &QLineEdit::editingFinished, this, &PropertiesDock::onSizeEditingFinished);
    connect(m_pickShapeButton, &QPushButton::clicked, this, &PropertiesDock::onPickShapeClicked);

    // Labor Class
    m_laborClassCombo = new QComboBox(m_container);
    m_formLayout->addRow("Labor:", m_laborClassCombo);
    connect(m_laborClassCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PropertiesDock::onLaborClassChanged);

    // Notes (multiline)
    m_notesEdit = new QTextEdit(m_container);
    m_notesEdit->setPlaceholderText("Enter notes...");
    m_notesEdit->setMaximumHeight(80);
    m_formLayout->addRow("Notes:", m_notesEdit);
    connect(m_notesEdit, &QTextEdit::textChanged, this, &PropertiesDock::onNotesChanged);

    // Spacer
    mainLayout->addStretch();

    // Populate combo boxes
    populateComboBoxes();

    setWidget(m_container);
    setMinimumWidth(220);
}

void PropertiesDock::populateComboBoxes()
{
    // Category
    for (const QString& cat : Measurement::categoryStrings()) {
        m_categoryCombo->addItem(cat);
    }

    // Material Type
    for (const QString& mat : Measurement::materialTypeStrings()) {
        m_materialTypeCombo->addItem(mat);
    }

    // Labor Class
    for (const QString& lab : Measurement::laborClassStrings()) {
        m_laborClassCombo->addItem(lab);
    }
}

void PropertiesDock::setFieldsEnabled(bool enabled)
{
    m_nameEdit->setEnabled(enabled);
    m_notesEdit->setEnabled(enabled);
    m_categoryCombo->setEnabled(enabled);
    m_materialTypeCombo->setEnabled(enabled);
    m_sizeEdit->setEnabled(enabled);
    m_pickShapeButton->setEnabled(enabled);
    m_laborClassCombo->setEnabled(enabled);
    m_infoLabel->setVisible(enabled);
}

void PropertiesDock::setMeasurement(const Measurement* measurement, int id)
{
    if (!measurement || id < 0) {
        clearSelection();
        return;
    }

    m_currentMeasurementId = id;
    m_noSelectionLabel->hide();
    setFieldsEnabled(true);

    updateFromMeasurement(measurement);
}

void PropertiesDock::clearSelection()
{
    m_currentMeasurementId = -1;
    m_noSelectionLabel->show();
    setFieldsEnabled(false);

    m_updatingFields = true;
    m_nameEdit->clear();
    m_notesEdit->clear();
    m_categoryCombo->setCurrentIndex(3);  // Misc
    m_materialTypeCombo->setCurrentIndex(5);  // Other
    m_sizeEdit->clear();
    m_laborClassCombo->setCurrentIndex(0);  // ShopFab
    m_infoLabel->clear();
    m_updatingFields = false;
}

void PropertiesDock::updateFromMeasurement(const Measurement* measurement)
{
    if (!measurement) {
        return;
    }

    m_updatingFields = true;

    // Update fields
    m_nameEdit->setText(measurement->name());
    m_notesEdit->setPlainText(measurement->notes());
    m_categoryCombo->setCurrentText(measurement->categoryString());
    m_materialTypeCombo->setCurrentText(measurement->materialTypeString());
    m_sizeEdit->setText(measurement->size());
    m_laborClassCombo->setCurrentText(measurement->laborClassString());

    // Info label
    m_infoLabel->setText(QString("%1: %2 in (%3 ft)")
        .arg(measurement->typeString())
        .arg(measurement->lengthInches(), 0, 'f', 2)
        .arg(measurement->lengthFeet(), 0, 'f', 2));

    // Cache values for change detection
    m_cachedName = measurement->name();
    m_cachedNotes = measurement->notes();
    m_cachedCategory = measurement->category();
    m_cachedMaterialType = measurement->materialType();
    m_cachedSize = measurement->size();
    m_cachedLaborClass = measurement->laborClass();

    m_updatingFields = false;
}

// ============================================================================
// Slots
// ============================================================================

void PropertiesDock::onNameEditingFinished()
{
    if (m_updatingFields || m_currentMeasurementId < 0) return;

    QString newValue = m_nameEdit->text();
    if (newValue != m_cachedName) {
        QString oldValue = m_cachedName;
        m_cachedName = newValue;
        emit nameChanged(m_currentMeasurementId, oldValue, newValue);
    }
}

void PropertiesDock::onNotesChanged()
{
    if (m_updatingFields || m_currentMeasurementId < 0) return;

    QString newValue = m_notesEdit->toPlainText();
    if (newValue != m_cachedNotes) {
        QString oldValue = m_cachedNotes;
        m_cachedNotes = newValue;
        emit notesChanged(m_currentMeasurementId, oldValue, newValue);
    }
}

void PropertiesDock::onCategoryChanged(int index)
{
    if (m_updatingFields || m_currentMeasurementId < 0) return;

    Category newValue = Measurement::categoryFromString(m_categoryCombo->currentText());
    if (newValue != m_cachedCategory) {
        Category oldValue = m_cachedCategory;
        m_cachedCategory = newValue;
        emit categoryChanged(m_currentMeasurementId, oldValue, newValue);
    }
}

void PropertiesDock::onMaterialTypeChanged(int index)
{
    if (m_updatingFields || m_currentMeasurementId < 0) return;

    MaterialType newValue = Measurement::materialTypeFromString(m_materialTypeCombo->currentText());
    if (newValue != m_cachedMaterialType) {
        MaterialType oldValue = m_cachedMaterialType;
        m_cachedMaterialType = newValue;
        emit materialTypeChanged(m_currentMeasurementId, oldValue, newValue);
    }
}

void PropertiesDock::onSizeEditingFinished()
{
    if (m_updatingFields || m_currentMeasurementId < 0) return;

    QString newValue = m_sizeEdit->text();
    if (newValue != m_cachedSize) {
        QString oldValue = m_cachedSize;
        m_cachedSize = newValue;
        emit sizeChanged(m_currentMeasurementId, oldValue, newValue);
    }
}

void PropertiesDock::onLaborClassChanged(int index)
{
    if (m_updatingFields || m_currentMeasurementId < 0) return;

    LaborClass newValue = Measurement::laborClassFromString(m_laborClassCombo->currentText());
    if (newValue != m_cachedLaborClass) {
        LaborClass oldValue = m_cachedLaborClass;
        m_cachedLaborClass = newValue;
        emit laborClassChanged(m_currentMeasurementId, oldValue, newValue);
    }
}

void PropertiesDock::focusSizeField()
{
    m_sizeEdit->setFocus();
    m_sizeEdit->selectAll();
}

void PropertiesDock::onPickShapeClicked()
{
    if (m_currentMeasurementId >= 0) {
        emit pickShapeRequested(m_currentMeasurementId);
    }
}

