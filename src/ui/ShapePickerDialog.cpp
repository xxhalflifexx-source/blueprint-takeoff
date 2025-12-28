#include "ShapePickerDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QTimer>

ShapePickerDialog::ShapePickerDialog(ProjectDatabase* db, QWidget* parent)
    : QDialog(parent)
    , m_db(db)
    , m_typeFilter(nullptr)
    , m_searchBox(nullptr)
    , m_table(nullptr)
    , m_okButton(nullptr)
    , m_cancelButton(nullptr)
    , m_statusLabel(nullptr)
    , m_selectedId(-1)
    , m_selectedWeight(0.0)
{
    setupUi();
    populateTypeFilter();
    refreshTable();
}

ShapePickerDialog::~ShapePickerDialog()
{
}

void ShapePickerDialog::setupUi()
{
    setWindowTitle("Pick AISC Shape");
    setMinimumSize(600, 400);
    resize(700, 500);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Filter row
    QHBoxLayout* filterLayout = new QHBoxLayout();

    filterLayout->addWidget(new QLabel("Type:", this));
    m_typeFilter = new QComboBox(this);
    m_typeFilter->setMinimumWidth(100);
    filterLayout->addWidget(m_typeFilter);

    filterLayout->addSpacing(20);

    filterLayout->addWidget(new QLabel("Search:", this));
    m_searchBox = new QLineEdit(this);
    m_searchBox->setPlaceholderText("Filter by designation...");
    m_searchBox->setClearButtonEnabled(true);
    filterLayout->addWidget(m_searchBox, 1);

    mainLayout->addLayout(filterLayout);

    // Table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(3);
    m_table->setHorizontalHeaderLabels({"Designation", "Type", "W (lb/ft)"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    mainLayout->addWidget(m_table, 1);

    // Status label
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: gray;");
    mainLayout->addWidget(m_statusLabel);

    // Button row
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    m_okButton = new QPushButton("OK", this);
    m_okButton->setEnabled(false);
    m_okButton->setDefault(true);
    buttonLayout->addWidget(m_okButton);

    m_cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(m_typeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ShapePickerDialog::onTypeFilterChanged);
    
    // Use a timer to debounce search input
    QTimer* searchTimer = new QTimer(this);
    searchTimer->setSingleShot(true);
    searchTimer->setInterval(300);
    connect(m_searchBox, &QLineEdit::textChanged, [searchTimer]() {
        searchTimer->start();
    });
    connect(searchTimer, &QTimer::timeout, this, &ShapePickerDialog::onSearch);

    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &ShapePickerDialog::onTableSelectionChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked,
            this, &ShapePickerDialog::onTableDoubleClicked);

    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ShapePickerDialog::populateTypeFilter()
{
    m_typeFilter->blockSignals(true);
    m_typeFilter->clear();
    m_typeFilter->addItem("All Types", QString());

    if (m_db && m_db->isOpen()) {
        QStringList types = m_db->getShapeTypes();
        for (const QString& type : types) {
            m_typeFilter->addItem(type, type);
        }
    }

    m_typeFilter->blockSignals(false);
}

void ShapePickerDialog::refreshTable()
{
    m_table->setRowCount(0);
    m_selectedId = -1;
    m_selectedLabel.clear();
    m_selectedWeight = 0.0;
    m_okButton->setEnabled(false);

    if (!m_db || !m_db->isOpen()) {
        m_statusLabel->setText("Database not available");
        return;
    }

    QString typeFilter = m_typeFilter->currentData().toString();
    QString searchText = m_searchBox->text().trimmed();

    QVector<ProjectDatabase::Shape> shapes = m_db->searchShapes(searchText, typeFilter, 500);

    m_table->setRowCount(shapes.size());

    for (int i = 0; i < shapes.size(); ++i) {
        const ProjectDatabase::Shape& shape = shapes[i];

        QTableWidgetItem* desigItem = new QTableWidgetItem(shape.designation);
        desigItem->setData(Qt::UserRole, shape.id);
        desigItem->setData(Qt::UserRole + 1, shape.wLbPerFt);
        m_table->setItem(i, 0, desigItem);

        m_table->setItem(i, 1, new QTableWidgetItem(shape.shapeType));
        m_table->setItem(i, 2, new QTableWidgetItem(
            shape.wLbPerFt > 0 ? QString::number(shape.wLbPerFt, 'f', 2) : ""));
    }

    int total = m_db->getShapeCount();
    if (shapes.size() < total) {
        m_statusLabel->setText(QString("Showing %1 of %2 shapes").arg(shapes.size()).arg(total));
    } else {
        m_statusLabel->setText(QString("%1 shapes").arg(shapes.size()));
    }
}

void ShapePickerDialog::onSearch()
{
    refreshTable();
}

void ShapePickerDialog::onTypeFilterChanged(int index)
{
    Q_UNUSED(index);
    refreshTable();
}

void ShapePickerDialog::onTableSelectionChanged()
{
    QList<QTableWidgetItem*> selected = m_table->selectedItems();
    if (selected.isEmpty()) {
        m_selectedId = -1;
        m_selectedLabel.clear();
        m_selectedWeight = 0.0;
        m_okButton->setEnabled(false);
        return;
    }

    // Get the first column item of the selected row
    int row = selected.first()->row();
    QTableWidgetItem* desigItem = m_table->item(row, 0);
    
    if (desigItem) {
        m_selectedId = desigItem->data(Qt::UserRole).toInt();
        m_selectedLabel = desigItem->text();
        m_selectedWeight = desigItem->data(Qt::UserRole + 1).toDouble();
        m_okButton->setEnabled(true);
    }
}

void ShapePickerDialog::onTableDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    QTableWidgetItem* desigItem = m_table->item(row, 0);
    if (desigItem) {
        m_selectedId = desigItem->data(Qt::UserRole).toInt();
        m_selectedLabel = desigItem->text();
        m_selectedWeight = desigItem->data(Qt::UserRole + 1).toDouble();
        accept();
    }
}

int ShapePickerDialog::selectedShapeId() const
{
    return m_selectedId;
}

QString ShapePickerDialog::selectedShapeLabel() const
{
    return m_selectedLabel;
}

double ShapePickerDialog::selectedShapeWeight() const
{
    return m_selectedWeight;
}
