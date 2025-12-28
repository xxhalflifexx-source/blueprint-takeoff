#ifndef QUOTECALCULATOR_H
#define QUOTECALCULATOR_H

#include <QVector>
#include <QString>
#include "Measurement.h"
#include "Project.h"

// Forward declaration
class ShapesDatabase;

/**
 * @brief A single line in the quote summary (grouped by material+size+labor).
 */
struct QuoteLineItem
{
    MaterialType materialType;
    QString size;
    LaborClass laborClass;
    
    double totalLengthInches = 0.0;
    double totalLengthFeet = 0.0;
    int itemCount = 0;
    
    // Weight (if AISC shapes are used)
    double totalWeightLb = 0.0;
    
    // Calculated costs (need rates to compute)
    double materialCost = 0.0;
    double laborCost = 0.0;
    double subtotal = 0.0;
    
    // For display
    QString materialTypeString() const;
    QString laborClassString() const;
};

/**
 * @brief Full quote summary with totals.
 */
struct QuoteSummary
{
    QVector<QuoteLineItem> lineItems;
    
    double grandSubtotal = 0.0;     // Sum of all line subtotals
    double grandTotal = 0.0;        // After markup
    double totalMaterialCost = 0.0;
    double totalLaborCost = 0.0;
    double grandTotalWeight = 0.0;  // Total weight in lbs
};

/**
 * @brief Calculates quote summaries from measurements.
 * 
 * Groups measurements by MaterialType + Size + LaborClass and computes totals.
 */
class QuoteCalculator
{
public:
    QuoteCalculator();

    /**
     * @brief Calculate the quote summary from project measurements.
     * @param measurements List of measurements
     * @param rates Quote rates for pricing
     * @param shapesDb Optional shapes database for weight lookup
     * @return QuoteSummary with grouped line items and totals
     */
    QuoteSummary calculate(const QVector<Measurement>& measurements, 
                           const QuoteRates& rates,
                           ShapesDatabase* shapesDb = nullptr) const;

private:
    // Generate a unique key for grouping
    QString groupKey(MaterialType material, const QString& size, LaborClass labor) const;
};

#endif // QUOTECALCULATOR_H

