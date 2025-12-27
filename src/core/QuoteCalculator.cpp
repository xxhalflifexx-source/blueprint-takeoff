#include "QuoteCalculator.h"
#include <QMap>

// ============================================================================
// QuoteLineItem
// ============================================================================

QString QuoteLineItem::materialTypeString() const
{
    switch (materialType) {
        case MaterialType::Tube:    return "Tube";
        case MaterialType::Angle:   return "Angle";
        case MaterialType::Channel: return "Channel";
        case MaterialType::FlatBar: return "FlatBar";
        case MaterialType::Plate:   return "Plate";
        case MaterialType::Other:   return "Other";
        default:                    return "Other";
    }
}

QString QuoteLineItem::laborClassString() const
{
    switch (laborClass) {
        case LaborClass::ShopFab:      return "ShopFab";
        case LaborClass::FieldInstall: return "FieldInstall";
        case LaborClass::FieldWeld:    return "FieldWeld";
        default:                       return "ShopFab";
    }
}

// ============================================================================
// QuoteCalculator
// ============================================================================

QuoteCalculator::QuoteCalculator()
{
}

QuoteSummary QuoteCalculator::calculate(const QVector<Measurement>& measurements,
                                        const QuoteRates& rates) const
{
    QuoteSummary summary;
    
    // Group measurements by (MaterialType, Size, LaborClass)
    QMap<QString, QuoteLineItem> groups;
    
    for (const Measurement& m : measurements) {
        // Only include Line and Polyline measurements
        if (m.type() != MeasurementType::Line && m.type() != MeasurementType::Polyline) {
            continue;
        }
        
        QString key = groupKey(m.materialType(), m.size(), m.laborClass());
        
        if (!groups.contains(key)) {
            QuoteLineItem item;
            item.materialType = m.materialType();
            item.size = m.size();
            item.laborClass = m.laborClass();
            groups[key] = item;
        }
        
        groups[key].totalLengthInches += m.lengthInches();
        groups[key].itemCount++;
    }
    
    // Calculate costs and build line items
    double totalMaterial = 0.0;
    double totalLabor = 0.0;
    
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        QuoteLineItem& item = it.value();
        
        // Convert to feet
        item.totalLengthFeet = item.totalLengthInches / 12.0;
        
        // Calculate costs
        item.materialCost = item.totalLengthFeet * rates.materialRatePerFt;
        item.laborCost = item.totalLengthFeet * rates.laborRatePerFt;
        item.subtotal = item.materialCost + item.laborCost;
        
        totalMaterial += item.materialCost;
        totalLabor += item.laborCost;
        
        summary.lineItems.append(item);
    }
    
    // Calculate totals
    summary.totalMaterialCost = totalMaterial;
    summary.totalLaborCost = totalLabor;
    summary.grandSubtotal = totalMaterial + totalLabor;
    summary.grandTotal = summary.grandSubtotal * (1.0 + rates.markupPercent / 100.0);
    
    return summary;
}

QString QuoteCalculator::groupKey(MaterialType material, const QString& size, LaborClass labor) const
{
    return QString("%1|%2|%3")
        .arg(static_cast<int>(material))
        .arg(size)
        .arg(static_cast<int>(labor));
}

