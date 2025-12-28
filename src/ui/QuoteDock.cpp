#include "QuoteDock.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QMap>

QuoteDock::QuoteDock(QWidget* parent)
    : QDockWidget("Quote Summary", parent)
    , m_container(nullptr)
    , m_table(nullptr)
    , m_currentPageOnlyCheck(nullptr)
    , m_pricePerLbSpin(nullptr)
    , m_totalWeightLabel(nullptr)
    , m_totalCostLabel(nullptr)
    , m_totalQtyLabel(nullptr)
    , m_exportButton(nullptr)
    , m_cachedProject(nullptr)
{
    setupUi();
}

QuoteDock::~QuoteDock()
{
}

void QuoteDock::setupUi()
{
    m_container = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(m_container);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Filter and price row
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    m_currentPageOnlyCheck = new QCheckBox("Current Page Only", m_container);
    m_currentPageOnlyCheck->setToolTip("Show quote summary for current page only");
    connect(m_currentPageOnlyCheck, &QCheckBox::toggled, 
            this, &QuoteDock::onCurrentPageOnlyToggled);
    topLayout->addWidget(m_currentPageOnlyCheck);
    
    topLayout->addStretch();

    // Material price per lb
    topLayout->addWidget(new QLabel("Material Rate:"));
    m_pricePerLbSpin = new QDoubleSpinBox(m_container);
    m_pricePerLbSpin->setRange(0, 999.99);
    m_pricePerLbSpin->setDecimals(2);
    m_pricePerLbSpin->setPrefix("$");
    m_pricePerLbSpin->setSuffix("/lb");
    m_pricePerLbSpin->setValue(0.50);
    m_pricePerLbSpin->setToolTip("Price per pound for material cost calculation");
    topLayout->addWidget(m_pricePerLbSpin);
    connect(m_pricePerLbSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &QuoteDock::onPriceChanged);

    mainLayout->addLayout(topLayout);

    // Table
    m_table = new QTableWidget(m_container);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "Designation", "Qty", "Total (ft)", "lb/ft", "Weight (lb)", "$/lb", "Cost ($)"
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    mainLayout->addWidget(m_table, 1);

    // Totals and export row
    QHBoxLayout* bottomLayout = new QHBoxLayout();

    m_totalQtyLabel = new QLabel("Items: 0", m_container);
    m_totalQtyLabel->setStyleSheet("font-weight: bold;");
    bottomLayout->addWidget(m_totalQtyLabel);

    bottomLayout->addSpacing(20);

    m_totalWeightLabel = new QLabel("Total Weight: 0 lb", m_container);
    m_totalWeightLabel->setStyleSheet("font-weight: bold;");
    bottomLayout->addWidget(m_totalWeightLabel);

    bottomLayout->addSpacing(20);

    m_totalCostLabel = new QLabel("Total Cost: $0.00", m_container);
    m_totalCostLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2a7;");
    bottomLayout->addWidget(m_totalCostLabel);

    bottomLayout->addStretch();

    m_exportButton = new QPushButton("Export CSV...", m_container);
    connect(m_exportButton, &QPushButton::clicked, this, &QuoteDock::onExportCsv);
    bottomLayout->addWidget(m_exportButton);

    mainLayout->addLayout(bottomLayout);

    setWidget(m_container);
}

void QuoteDock::updateFromProject(Project* project)
{
    m_cachedProject = project;

    if (!project || !project->isOpen()) {
        m_table->setRowCount(0);
        updateTotals(0, 0, 0);
        return;
    }

    QString pageFilter;
    if (m_currentPageOnlyCheck->isChecked() && !m_currentPageId.isEmpty()) {
        pageFilter = m_currentPageId;
    }

    populateTable(project, pageFilter);
}

double QuoteDock::materialPricePerLb() const
{
    return m_pricePerLbSpin->value();
}

void QuoteDock::setMaterialPricePerLb(double pricePerLb)
{
    m_pricePerLbSpin->blockSignals(true);
    m_pricePerLbSpin->setValue(pricePerLb);
    m_pricePerLbSpin->blockSignals(false);
}

bool QuoteDock::isCurrentPageOnly() const
{
    return m_currentPageOnlyCheck->isChecked();
}

void QuoteDock::populateTable(Project* project, const QString& pageFilter)
{
    // Group items by designation
    struct DesignationGroup {
        QString designation;
        int qty = 0;
        double totalLengthFt = 0.0;
        double wLbPerFt = 0.0;
        double totalWeightLb = 0.0;
        double totalCost = 0.0;
    };

    QMap<QString, DesignationGroup> groups;
    double pricePerLb = m_pricePerLbSpin->value();

    const QVector<TakeoffItem>& items = project->takeoffItems();
    for (const TakeoffItem& item : items) {
        // Filter by page if specified
        if (!pageFilter.isEmpty() && item.pageId() != pageFilter) {
            continue;
        }

        QString key = item.designation().isEmpty() ? "(Unassigned)" : item.designation();
        
        if (!groups.contains(key)) {
            groups[key].designation = key;
            // Get weight per foot from shape
            if (item.shapeId() > 0) {
                auto shape = project->getShape(item.shapeId());
                groups[key].wLbPerFt = shape.wLbPerFt;
            }
        }

        DesignationGroup& group = groups[key];
        group.qty += item.qty();
        double lengthFt = item.totalLengthFeet();
        group.totalLengthFt += lengthFt;
        
        if (group.wLbPerFt > 0) {
            double weight = lengthFt * group.wLbPerFt;
            group.totalWeightLb += weight;
            group.totalCost += weight * pricePerLb;
        }
    }

    // Populate table
    m_table->setRowCount(groups.size());
    
    double grandTotalWeight = 0.0;
    double grandTotalCost = 0.0;
    int grandTotalQty = 0;
    
    int row = 0;
    for (auto it = groups.begin(); it != groups.end(); ++it, ++row) {
        const DesignationGroup& group = it.value();
        
        m_table->setItem(row, 0, new QTableWidgetItem(group.designation));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::number(group.qty)));
        m_table->setItem(row, 2, new QTableWidgetItem(QString::number(group.totalLengthFt, 'f', 2)));
        m_table->setItem(row, 3, new QTableWidgetItem(
            group.wLbPerFt > 0 ? QString::number(group.wLbPerFt, 'f', 2) : "-"));
        m_table->setItem(row, 4, new QTableWidgetItem(
            group.totalWeightLb > 0 ? QString::number(group.totalWeightLb, 'f', 1) : "-"));
        m_table->setItem(row, 5, new QTableWidgetItem(QString::number(pricePerLb, 'f', 2)));
        m_table->setItem(row, 6, new QTableWidgetItem(
            group.totalCost > 0 ? QString::number(group.totalCost, 'f', 2) : "-"));
        
        grandTotalWeight += group.totalWeightLb;
        grandTotalCost += group.totalCost;
        grandTotalQty += group.qty;
    }

    updateTotals(grandTotalWeight, grandTotalCost, grandTotalQty);
}

void QuoteDock::updateTotals(double totalWeight, double totalCost, int totalQty)
{
    m_totalQtyLabel->setText(QString("Items: %1").arg(totalQty));
    m_totalWeightLabel->setText(QString("Total Weight: %1 lb").arg(totalWeight, 0, 'f', 1));
    m_totalCostLabel->setText(QString("Total Cost: $%1").arg(totalCost, 0, 'f', 2));
}

void QuoteDock::onExportCsv()
{
    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Export Quote Summary",
        QString(),
        "CSV Files (*.csv);;All Files (*)"
    );

    if (filePath.isEmpty()) {
        return;
    }

    // Ensure .csv extension
    if (!filePath.endsWith(".csv", Qt::CaseInsensitive)) {
        filePath += ".csv";
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export Error",
            QString("Cannot open file for writing: %1").arg(file.errorString()));
        return;
    }

    QTextStream out(&file);

    // Header
    out << "Designation,Qty,Total (ft),lb/ft,Weight (lb),$/lb,Cost ($)\n";

    // Data rows from table
    for (int row = 0; row < m_table->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < m_table->columnCount(); ++col) {
            QTableWidgetItem* item = m_table->item(row, col);
            QString text = item ? item->text() : "";
            // Escape fields that might contain commas
            if (text.contains(',') || text.contains('"')) {
                text = "\"" + text.replace("\"", "\"\"") + "\"";
            }
            rowData << text;
        }
        out << rowData.join(",") << "\n";
    }

    // Summary
    out << "\n";
    out << "Material Rate:,$" << QString::number(m_pricePerLbSpin->value(), 'f', 2) << "/lb\n";
    out << m_totalQtyLabel->text() << "\n";
    out << m_totalWeightLabel->text() << "\n";
    out << m_totalCostLabel->text() << "\n";

    file.close();

    QMessageBox::information(this, "Export Complete",
        QString("Quote summary exported to:\n%1").arg(filePath));
}

void QuoteDock::onPriceChanged()
{
    // Recalculate with new price
    if (m_cachedProject) {
        updateFromProject(m_cachedProject);
    }
    
    emit materialPriceChanged(m_pricePerLbSpin->value());
}

void QuoteDock::onCurrentPageOnlyToggled(bool checked)
{
    if (m_cachedProject) {
        updateFromProject(m_cachedProject);
    }
    
    emit currentPageOnlyChanged(checked);
}
