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

QuoteDock::QuoteDock(QWidget* parent)
    : QDockWidget("Quote Summary", parent)
    , m_container(nullptr)
    , m_table(nullptr)
    , m_currentPageOnlyCheck(nullptr)
    , m_materialRateSpin(nullptr)
    , m_laborRateSpin(nullptr)
    , m_markupSpin(nullptr)
    , m_subtotalLabel(nullptr)
    , m_totalLabel(nullptr)
    , m_weightLabel(nullptr)
    , m_exportButton(nullptr)
    , m_shapesDb(nullptr)
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

    // Filter row
    QHBoxLayout* filterLayout = new QHBoxLayout();
    m_currentPageOnlyCheck = new QCheckBox("Current Page Only", m_container);
    m_currentPageOnlyCheck->setToolTip("Show quote summary for current page only");
    connect(m_currentPageOnlyCheck, &QCheckBox::toggled, 
            this, &QuoteDock::onCurrentPageOnlyToggled);
    filterLayout->addWidget(m_currentPageOnlyCheck);
    filterLayout->addStretch();
    mainLayout->addLayout(filterLayout);

    // Table
    m_table = new QTableWidget(m_container);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({
        "Material", "Size", "Labor", "Total (in)", "Total (ft)", "Weight (lb)", "Count"
    });
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setAlternatingRowColors(true);
    mainLayout->addWidget(m_table, 1);

    // Rates group
    QGroupBox* ratesGroup = new QGroupBox("Rates", m_container);
    QHBoxLayout* ratesLayout = new QHBoxLayout(ratesGroup);

    // Material rate
    ratesLayout->addWidget(new QLabel("Material $/ft:"));
    m_materialRateSpin = new QDoubleSpinBox(ratesGroup);
    m_materialRateSpin->setRange(0, 9999.99);
    m_materialRateSpin->setDecimals(2);
    m_materialRateSpin->setPrefix("$");
    ratesLayout->addWidget(m_materialRateSpin);

    ratesLayout->addSpacing(20);

    // Labor rate
    ratesLayout->addWidget(new QLabel("Labor $/ft:"));
    m_laborRateSpin = new QDoubleSpinBox(ratesGroup);
    m_laborRateSpin->setRange(0, 9999.99);
    m_laborRateSpin->setDecimals(2);
    m_laborRateSpin->setPrefix("$");
    ratesLayout->addWidget(m_laborRateSpin);

    ratesLayout->addSpacing(20);

    // Markup
    ratesLayout->addWidget(new QLabel("Markup %:"));
    m_markupSpin = new QDoubleSpinBox(ratesGroup);
    m_markupSpin->setRange(0, 999.99);
    m_markupSpin->setDecimals(2);
    m_markupSpin->setSuffix("%");
    ratesLayout->addWidget(m_markupSpin);

    ratesLayout->addStretch();
    mainLayout->addWidget(ratesGroup);

    // Connect rate changes
    connect(m_materialRateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &QuoteDock::onRateChanged);
    connect(m_laborRateSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &QuoteDock::onRateChanged);
    connect(m_markupSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &QuoteDock::onRateChanged);

    // Totals and export row
    QHBoxLayout* bottomLayout = new QHBoxLayout();

    m_subtotalLabel = new QLabel("Subtotal: $0.00", m_container);
    m_subtotalLabel->setStyleSheet("font-weight: bold;");
    bottomLayout->addWidget(m_subtotalLabel);

    bottomLayout->addSpacing(30);

    m_totalLabel = new QLabel("Total: $0.00", m_container);
    m_totalLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    bottomLayout->addWidget(m_totalLabel);

    bottomLayout->addSpacing(30);

    m_weightLabel = new QLabel("Weight: 0 lb", m_container);
    m_weightLabel->setStyleSheet("font-weight: bold;");
    bottomLayout->addWidget(m_weightLabel);

    bottomLayout->addStretch();

    m_exportButton = new QPushButton("Export CSV...", m_container);
    connect(m_exportButton, &QPushButton::clicked, this, &QuoteDock::onExportCsv);
    bottomLayout->addWidget(m_exportButton);

    mainLayout->addLayout(bottomLayout);

    setWidget(m_container);
}

void QuoteDock::updateFromProject(const Project& project)
{
    updateFromMeasurements(project.measurements(), project.quoteRates());
}

void QuoteDock::updateFromMeasurements(const QVector<Measurement>& measurements, 
                                        const QuoteRates& rates,
                                        ShapesDatabase* shapesDb)
{
    m_cachedMeasurements = measurements;
    m_shapesDb = shapesDb;
    
    // Calculate summary
    QuoteSummary summary = m_calculator.calculate(m_cachedMeasurements, currentRates(), m_shapesDb);
    
    populateTable(summary);
    updateTotals(summary);
}

QuoteRates QuoteDock::currentRates() const
{
    QuoteRates rates;
    rates.materialRatePerFt = m_materialRateSpin->value();
    rates.laborRatePerFt = m_laborRateSpin->value();
    rates.markupPercent = m_markupSpin->value();
    return rates;
}

void QuoteDock::setRates(const QuoteRates& rates)
{
    // Block signals to prevent triggering ratesChanged
    m_materialRateSpin->blockSignals(true);
    m_laborRateSpin->blockSignals(true);
    m_markupSpin->blockSignals(true);

    m_materialRateSpin->setValue(rates.materialRatePerFt);
    m_laborRateSpin->setValue(rates.laborRatePerFt);
    m_markupSpin->setValue(rates.markupPercent);

    m_materialRateSpin->blockSignals(false);
    m_laborRateSpin->blockSignals(false);
    m_markupSpin->blockSignals(false);
}

bool QuoteDock::isCurrentPageOnly() const
{
    return m_currentPageOnlyCheck->isChecked();
}

void QuoteDock::populateTable(const QuoteSummary& summary)
{
    m_table->setRowCount(summary.lineItems.size());

    for (int i = 0; i < summary.lineItems.size(); ++i) {
        const QuoteLineItem& item = summary.lineItems[i];

        m_table->setItem(i, 0, new QTableWidgetItem(item.materialTypeString()));
        m_table->setItem(i, 1, new QTableWidgetItem(item.size));
        m_table->setItem(i, 2, new QTableWidgetItem(item.laborClassString()));
        m_table->setItem(i, 3, new QTableWidgetItem(QString::number(item.totalLengthInches, 'f', 2)));
        m_table->setItem(i, 4, new QTableWidgetItem(QString::number(item.totalLengthFeet, 'f', 2)));
        m_table->setItem(i, 5, new QTableWidgetItem(
            item.totalWeightLb > 0 ? QString::number(item.totalWeightLb, 'f', 1) : "-"));
        m_table->setItem(i, 6, new QTableWidgetItem(QString::number(item.itemCount)));
    }
}

void QuoteDock::updateTotals(const QuoteSummary& summary)
{
    m_subtotalLabel->setText(QString("Subtotal: $%1").arg(summary.grandSubtotal, 0, 'f', 2));
    m_totalLabel->setText(QString("Total: $%1").arg(summary.grandTotal, 0, 'f', 2));
    m_weightLabel->setText(QString("Weight: %1 lb").arg(summary.grandTotalWeight, 0, 'f', 1));
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
    out << "Material Type,Size,Labor Class,Total (in),Total (ft),Weight (lb),Item Count\n";

    // Data rows
    QuoteSummary summary = m_calculator.calculate(m_cachedMeasurements, currentRates(), m_shapesDb);
    for (const QuoteLineItem& item : summary.lineItems) {
        // Escape fields that might contain commas
        QString size = item.size;
        if (size.contains(',') || size.contains('"')) {
            size = "\"" + size.replace("\"", "\"\"") + "\"";
        }

        out << item.materialTypeString() << ","
            << size << ","
            << item.laborClassString() << ","
            << QString::number(item.totalLengthInches, 'f', 2) << ","
            << QString::number(item.totalLengthFeet, 'f', 2) << ","
            << QString::number(item.totalWeightLb, 'f', 1) << ","
            << item.itemCount << "\n";
    }

    // Summary row
    out << "\n";
    out << "Rates:,Material $/ft," << m_materialRateSpin->value() << "\n";
    out << ",Labor $/ft," << m_laborRateSpin->value() << "\n";
    out << ",Markup %," << m_markupSpin->value() << "\n";
    out << "\n";
    out << "Subtotal:,$" << QString::number(summary.grandSubtotal, 'f', 2) << "\n";
    out << "Total:,$" << QString::number(summary.grandTotal, 'f', 2) << "\n";
    out << "Total Weight:," << QString::number(summary.grandTotalWeight, 'f', 1) << " lb\n";

    file.close();

    QMessageBox::information(this, "Export Complete",
        QString("Quote summary exported to:\n%1").arg(filePath));
}

void QuoteDock::onRateChanged()
{
    // Recalculate with new rates
    QuoteSummary summary = m_calculator.calculate(m_cachedMeasurements, currentRates(), m_shapesDb);
    updateTotals(summary);

    emit ratesChanged(currentRates());
}

void QuoteDock::onCurrentPageOnlyToggled(bool checked)
{
    emit currentPageOnlyChanged(checked);
}
