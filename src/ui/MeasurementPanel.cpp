#include "MeasurementPanel.h"

MeasurementPanel::MeasurementPanel(QWidget* parent)
    : QWidget(parent)
    , m_layout(nullptr)
    , m_titleLabel(nullptr)
    , m_listWidget(nullptr)
{
    setupUi();
}

MeasurementPanel::~MeasurementPanel()
{
    // Qt handles child deletion
}

void MeasurementPanel::setupUi()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(5, 5, 5, 5);

    // Title label
    m_titleLabel = new QLabel("Measurements", this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_layout->addWidget(m_titleLabel);

    // List widget
    m_listWidget = new QListWidget(this);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listWidget->setMinimumWidth(180);
    m_layout->addWidget(m_listWidget);

    // Connect selection change signal
    connect(m_listWidget, &QListWidget::itemSelectionChanged,
            this, &MeasurementPanel::onSelectionChanged);

    setLayout(m_layout);
}

void MeasurementPanel::addMeasurement(const Measurement& measurement)
{
    QListWidgetItem* item = new QListWidgetItem(measurement.displayString(), m_listWidget);
    
    // Store mapping from item to measurement ID
    m_itemToId[item] = measurement.id();
    m_idToItem[measurement.id()] = item;
    
    // Set tooltip with more details
    QString tooltip = QString("ID: %1\nType: %2\nLength: %3 inches\nPoints: %4")
        .arg(measurement.id())
        .arg(measurement.typeString())
        .arg(measurement.lengthInches(), 0, 'f', 2)
        .arg(measurement.points().size());
    item->setToolTip(tooltip);
}

void MeasurementPanel::updateMeasurement(const Measurement& measurement)
{
    if (!m_idToItem.contains(measurement.id())) {
        return;
    }
    
    QListWidgetItem* item = m_idToItem[measurement.id()];
    
    // Update display text
    item->setText(measurement.displayString());
    
    // Update tooltip
    QString tooltip = QString("ID: %1\nType: %2\nLength: %3 inches\nPoints: %4\nName: %5\nCategory: %6\nMaterial: %7")
        .arg(measurement.id())
        .arg(measurement.typeString())
        .arg(measurement.lengthInches(), 0, 'f', 2)
        .arg(measurement.points().size())
        .arg(measurement.name().isEmpty() ? "(none)" : measurement.name())
        .arg(measurement.categoryString())
        .arg(measurement.materialTypeString());
    item->setToolTip(tooltip);
}

void MeasurementPanel::removeMeasurement(int measurementId)
{
    if (!m_idToItem.contains(measurementId)) {
        return;
    }
    
    QListWidgetItem* item = m_idToItem[measurementId];
    
    // Remove from maps
    m_itemToId.remove(item);
    m_idToItem.remove(measurementId);
    
    // Remove from list widget
    int row = m_listWidget->row(item);
    if (row >= 0) {
        delete m_listWidget->takeItem(row);
    }
}

void MeasurementPanel::clearMeasurements()
{
    m_listWidget->clear();
    m_itemToId.clear();
    m_idToItem.clear();
}

int MeasurementPanel::selectedMeasurementId() const
{
    QList<QListWidgetItem*> selected = m_listWidget->selectedItems();
    if (selected.isEmpty()) {
        return -1;
    }
    
    QListWidgetItem* item = selected.first();
    return m_itemToId.value(item, -1);
}

void MeasurementPanel::selectMeasurement(int measurementId)
{
    if (!m_idToItem.contains(measurementId)) {
        return;
    }
    QListWidgetItem* item = m_idToItem[measurementId];
    m_listWidget->setCurrentItem(item);
    m_listWidget->scrollToItem(item);
    // Selection change signal will fire automatically
}

void MeasurementPanel::onSelectionChanged()
{
    int id = selectedMeasurementId();
    emit measurementSelected(id);
}
