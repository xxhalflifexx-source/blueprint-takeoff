#include "PdfImportDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QButtonGroup>
#include <QFileInfo>

PdfImportDialog::PdfImportDialog(const QString& pdfPath, int totalPages, QWidget* parent)
    : QDialog(parent)
    , m_allPagesRadio(nullptr)
    , m_rangeRadio(nullptr)
    , m_fromSpinBox(nullptr)
    , m_toSpinBox(nullptr)
    , m_totalPages(totalPages)
{
    setupUi(pdfPath, totalPages);
}

void PdfImportDialog::setupUi(const QString& pdfPath, int totalPages)
{
    setWindowTitle("Import PDF Pages");
    setMinimumWidth(350);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // File info
    QFileInfo fi(pdfPath);
    QLabel* fileLabel = new QLabel(QString("<b>File:</b> %1").arg(fi.fileName()), this);
    fileLabel->setWordWrap(true);
    mainLayout->addWidget(fileLabel);

    QLabel* pagesLabel = new QLabel(QString("<b>Total pages:</b> %1").arg(totalPages), this);
    mainLayout->addWidget(pagesLabel);

    mainLayout->addSpacing(10);

    // Selection options
    QLabel* selectLabel = new QLabel("Select pages to import:", this);
    mainLayout->addWidget(selectLabel);

    // Radio buttons
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    
    m_allPagesRadio = new QRadioButton(QString("All pages (1-%1)").arg(totalPages), this);
    m_allPagesRadio->setChecked(true);
    buttonGroup->addButton(m_allPagesRadio);
    mainLayout->addWidget(m_allPagesRadio);

    // Range option
    QHBoxLayout* rangeLayout = new QHBoxLayout();
    
    m_rangeRadio = new QRadioButton("Page range:", this);
    buttonGroup->addButton(m_rangeRadio);
    rangeLayout->addWidget(m_rangeRadio);

    QLabel* fromLabel = new QLabel("From", this);
    rangeLayout->addWidget(fromLabel);

    m_fromSpinBox = new QSpinBox(this);
    m_fromSpinBox->setMinimum(1);
    m_fromSpinBox->setMaximum(totalPages);
    m_fromSpinBox->setValue(1);
    m_fromSpinBox->setEnabled(false);
    rangeLayout->addWidget(m_fromSpinBox);

    QLabel* toLabel = new QLabel("to", this);
    rangeLayout->addWidget(toLabel);

    m_toSpinBox = new QSpinBox(this);
    m_toSpinBox->setMinimum(1);
    m_toSpinBox->setMaximum(totalPages);
    m_toSpinBox->setValue(totalPages);
    m_toSpinBox->setEnabled(false);
    rangeLayout->addWidget(m_toSpinBox);

    rangeLayout->addStretch();
    mainLayout->addLayout(rangeLayout);

    mainLayout->addSpacing(10);

    // Info label
    QLabel* infoLabel = new QLabel(
        "Each selected page will be imported as a separate page in the project.",
        this
    );
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: gray; font-size: 10pt;");
    mainLayout->addWidget(infoLabel);

    mainLayout->addStretch();

    // Button box
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        this
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Connect radio buttons to enable/disable spin boxes
    connect(m_allPagesRadio, &QRadioButton::toggled, this, &PdfImportDialog::onAllPagesToggled);

    // Validate range when spin boxes change
    connect(m_fromSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (m_toSpinBox->value() < value) {
            m_toSpinBox->setValue(value);
        }
    });
    connect(m_toSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value) {
        if (m_fromSpinBox->value() > value) {
            m_fromSpinBox->setValue(value);
        }
    });
}

void PdfImportDialog::onAllPagesToggled(bool checked)
{
    m_fromSpinBox->setEnabled(!checked);
    m_toSpinBox->setEnabled(!checked);
}

bool PdfImportDialog::isAllPages() const
{
    return m_allPagesRadio->isChecked();
}

int PdfImportDialog::fromPage() const
{
    if (isAllPages()) {
        return 1;
    }
    return m_fromSpinBox->value();
}

int PdfImportDialog::toPage() const
{
    if (isAllPages()) {
        return m_totalPages;
    }
    return m_toSpinBox->value();
}

int PdfImportDialog::totalPages() const
{
    return m_totalPages;
}

