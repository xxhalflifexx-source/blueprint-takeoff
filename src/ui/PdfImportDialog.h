#ifndef PDFIMPORTDIALOG_H
#define PDFIMPORTDIALOG_H

#include <QDialog>
#include <QString>

class QSpinBox;
class QRadioButton;
class QLabel;

/**
 * @brief Dialog for selecting which pages to import from a PDF.
 * 
 * Shows the total page count and allows the user to select:
 * - All pages
 * - A specific page range (From X to Y)
 */
class PdfImportDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructor.
     * @param pdfPath Path to the PDF file (for display)
     * @param totalPages Total number of pages in the PDF
     * @param parent Parent widget
     */
    explicit PdfImportDialog(const QString& pdfPath, int totalPages, QWidget* parent = nullptr);
    ~PdfImportDialog() override = default;

    /**
     * @brief Check if "All pages" is selected.
     * @return true if all pages should be imported
     */
    bool isAllPages() const;

    /**
     * @brief Get the starting page number (1-based).
     * @return First page to import
     */
    int fromPage() const;

    /**
     * @brief Get the ending page number (1-based).
     * @return Last page to import
     */
    int toPage() const;

    /**
     * @brief Get the total number of pages.
     * @return Total page count
     */
    int totalPages() const;

private slots:
    void onAllPagesToggled(bool checked);

private:
    void setupUi(const QString& pdfPath, int totalPages);

    QRadioButton* m_allPagesRadio;
    QRadioButton* m_rangeRadio;
    QSpinBox* m_fromSpinBox;
    QSpinBox* m_toSpinBox;
    int m_totalPages;
};

#endif // PDFIMPORTDIALOG_H

