#include "PropertiesDock.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

PropertiesDock::PropertiesDock(QWidget* parent)
    : QDockWidget("Properties", parent)
    , m_container(nullptr)
    , m_formLayout(nullptr)
    , m_noSelectionLabel(nullptr)
    , m_designationEdit(nullptr)
    , m_designationCompleter(nullptr)
    , m_designationModel(nullptr)
    , m_pickShapeButton(nullptr)
    , m_qtySpinBox(nullptr)
    , m_notesEdit(nullptr)
    , m_infoLabel(nullptr)
    , m_lengthLabel(nullptr)
    , m_weightLabel(nullptr)
    , m_costLabel(nullptr)
    , m_currentItemId(-1)
    , m_currentShapeId(-1)
    , m_cachedQty(1)
    , m_cachedLengthFeet(0.0)
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
    m_noSelectionLabel = new QLabel("No item selected", m_container);
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

    // Designation with autocomplete and Pick button
    QWidget* designationWidget = new QWidget(m_container);
    QHBoxLayout* designationLayout = new QHBoxLayout(designationWidget);
    designationLayout->setContentsMargins(0, 0, 0, 0);
    designationLayout->setSpacing(4);
    
    m_designationEdit = new QLineEdit(designationWidget);
    m_designationEdit->setPlaceholderText("e.g. W14X90");
    
    // Setup autocomplete
    m_designationModel = new QStringListModel(this);
    m_designationCompleter = new QCompleter(m_designationModel, this);
    m_designationCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_designationCompleter->setFilterMode(Qt::MatchContains);
    m_designationCompleter->setMaxVisibleItems(10);
    m_designationEdit->setCompleter(m_designationCompleter);
    
    designationLayout->addWidget(m_designationEdit, 1);
    
    m_pickShapeButton = new QPushButton("Pick...", designationWidget);
    m_pickShapeButton->setToolTip("Pick from AISC shapes database");
    m_pickShapeButton->setMaximumWidth(50);
    designationLayout->addWidget(m_pickShapeButton);
    
    m_formLayout->addRow("Designation:", designationWidget);
    connect(m_designationEdit, &QLineEdit::editingFinished, 
            this, &PropertiesDock::onDesignationEditingFinished);
    connect(m_designationCompleter, QOverload<const QString&>::of(&QCompleter::activated),
            this, &PropertiesDock::onDesignationSelected);
    connect(m_pickShapeButton, &QPushButton::clicked, 
            this, &PropertiesDock::onPickShapeClicked);

    // Qty
    m_qtySpinBox = new QSpinBox(m_container);
    m_qtySpinBox->setMinimum(1);
    m_qtySpinBox->setMaximum(9999);
    m_qtySpinBox->setValue(1);
    m_formLayout->addRow("Qty:", m_qtySpinBox);
    connect(m_qtySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PropertiesDock::onQtyValueChanged);

    // Notes (multiline)
    m_notesEdit = new QTextEdit(m_container);
    m_notesEdit->setPlaceholderText("Enter notes...");
    m_notesEdit->setMaximumHeight(60);
    m_formLayout->addRow("Notes:", m_notesEdit);
    connect(m_notesEdit, &QTextEdit::textChanged, this, &PropertiesDock::onNotesChanged);

    // Separator line
    QFrame* line = new QFrame(m_container);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // Computed values section
    QLabel* computedHeader = new QLabel("Computed Values", m_container);
    computedHeader->setStyleSheet("font-weight: bold; color: #555;");
    mainLayout->addWidget(computedHeader);

    QFormLayout* computedLayout = new QFormLayout();
    mainLayout->addLayout(computedLayout);

    m_lengthLabel = new QLabel("--", m_container);
    computedLayout->addRow("Length:", m_lengthLabel);

    m_weightLabel = new QLabel("--", m_container);
    computedLayout->addRow("Weight:", m_weightLabel);

    m_costLabel = new QLabel("--", m_container);
    m_costLabel->setStyleSheet("font-weight: bold; color: #2a7;");
    computedLayout->addRow("Material Cost:", m_costLabel);

    // Spacer
    mainLayout->addStretch();

    setWidget(m_container);
    setMinimumWidth(220);
}

void PropertiesDock::setFieldsEnabled(bool enabled)
{
    m_designationEdit->setEnabled(enabled);
    m_pickShapeButton->setEnabled(enabled);
    m_qtySpinBox->setEnabled(enabled);
    m_notesEdit->setEnabled(enabled);
    m_infoLabel->setVisible(enabled);
    m_lengthLabel->setVisible(enabled);
    m_weightLabel->setVisible(enabled);
    m_costLabel->setVisible(enabled);
}

void PropertiesDock::setTakeoffItem(const TakeoffItem* item, int id)
{
    if (!item || id < 0) {
        clearSelection();
        return;
    }

    m_currentItemId = id;
    m_currentShapeId = item->shapeId();
    m_noSelectionLabel->hide();
    setFieldsEnabled(true);

    updateFromItem(item);
}

void PropertiesDock::clearSelection()
{
    m_currentItemId = -1;
    m_currentShapeId = -1;
    m_noSelectionLabel->show();
    setFieldsEnabled(false);

    m_updatingFields = true;
    m_designationEdit->clear();
    m_qtySpinBox->setValue(1);
    m_notesEdit->clear();
    m_infoLabel->clear();
    m_lengthLabel->setText("--");
    m_weightLabel->setText("--");
    m_costLabel->setText("--");
    m_updatingFields = false;
}

void PropertiesDock::updateFromItem(const TakeoffItem* item)
{
    if (!item) {
        return;
    }

    m_updatingFields = true;

    // Update fields
    m_designationEdit->setText(item->designation());
    m_qtySpinBox->setValue(item->qty());
    m_notesEdit->setPlainText(item->notes());

    // Info label
    m_infoLabel->setText(QString("%1: %2 in (%3 ft)")
        .arg(item->kindString())
        .arg(item->lengthInches(), 0, 'f', 2)
        .arg(item->lengthFeet(), 0, 'f', 2));

    // Cache values for change detection
    m_cachedDesignation = item->designation();
    m_cachedQty = item->qty();
    m_cachedNotes = item->notes();
    m_cachedLengthFeet = item->lengthFeet();
    m_currentShapeId = item->shapeId();

    // Basic length display
    double totalFt = item->totalLengthFeet();
    m_lengthLabel->setText(QString("%1 ft (x%2)")
        .arg(totalFt, 0, 'f', 2)
        .arg(item->qty()));

    m_updatingFields = false;
}

void PropertiesDock::updateComputedValues(double wLbPerFt, double pricePerLb)
{
    if (m_currentItemId < 0) {
        return;
    }

    double totalFt = m_cachedLengthFeet * m_cachedQty;
    m_lengthLabel->setText(QString("%1 ft (x%2)")
        .arg(totalFt, 0, 'f', 2)
        .arg(m_cachedQty));

    if (wLbPerFt > 0) {
        double weight = totalFt * wLbPerFt;
        m_weightLabel->setText(QString("%1 lb @ %2 lb/ft")
            .arg(weight, 0, 'f', 1)
            .arg(wLbPerFt, 0, 'f', 2));

        if (pricePerLb > 0) {
            double cost = weight * pricePerLb;
            m_costLabel->setText(QString("$%1 @ $%2/lb")
                .arg(cost, 0, 'f', 2)
                .arg(pricePerLb, 0, 'f', 2));
        } else {
            m_costLabel->setText("Set $/lb in Quote");
        }
    } else {
        m_weightLabel->setText("Assign shape for weight");
        m_costLabel->setText("--");
    }
}

void PropertiesDock::focusDesignationField()
{
    m_designationEdit->setFocus();
    m_designationEdit->selectAll();
}

void PropertiesDock::setDesignationList(const QStringList& designations)
{
    m_designationModel->setStringList(designations);
}

// ============================================================================
// Slots
// ============================================================================

void PropertiesDock::onDesignationEditingFinished()
{
    if (m_updatingFields || m_currentItemId < 0) return;

    QString newValue = m_designationEdit->text().trimmed().toUpper();
    m_designationEdit->setText(newValue);  // Normalize to uppercase

    if (newValue != m_cachedDesignation) {
        QString oldValue = m_cachedDesignation;
        int oldShapeId = m_currentShapeId;
        m_cachedDesignation = newValue;
        // Shape ID will be resolved by MainWindow
        emit designationChanged(m_currentItemId, oldValue, newValue, oldShapeId, -1);
    }
}

void PropertiesDock::onDesignationSelected(const QString& text)
{
    // When autocomplete selection is made, treat as editing finished
    onDesignationEditingFinished();
}

void PropertiesDock::onQtyValueChanged(int value)
{
    if (m_updatingFields || m_currentItemId < 0) return;

    if (value != m_cachedQty) {
        int oldValue = m_cachedQty;
        m_cachedQty = value;
        emit qtyChanged(m_currentItemId, oldValue, value);
    }
}

void PropertiesDock::onNotesChanged()
{
    if (m_updatingFields || m_currentItemId < 0) return;

    QString newValue = m_notesEdit->toPlainText();
    if (newValue != m_cachedNotes) {
        QString oldValue = m_cachedNotes;
        m_cachedNotes = newValue;
        emit notesChanged(m_currentItemId, oldValue, newValue);
    }
}

void PropertiesDock::onPickShapeClicked()
{
    if (m_currentItemId >= 0) {
        emit pickShapeRequested(m_currentItemId);
    }
}
