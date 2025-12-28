#include "QuoteCalculator.h"
#include "ShapesDatabase.h"
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
                                        const QuoteRates& rates,
                                        ShapesDatabase* shapesDb) const
{
    QuoteSummary summary;
    
    // Group measurements by (MaterialType, Size, LaborClass)
    // Also track shapeId for weight calculations
    struct GroupData {
        QuoteLineItem item;
        QMap<int, double> shapeIdLengths;  // shapeId -> total length in feet for that shape
    };
    QMap<QString, GroupData> groups;
    
    for (const Measurement& m : measurements) {
        // Only include Line and Polyline measurements
        if (m.type() != MeasurementType::Line && m.type() != MeasurementType::Polyline) {
            continue;
        }
        
        QString key = groupKey(m.materialType(), m.size(), m.laborClass());
        
        if (!groups.contains(key)) {
            GroupData data;
            data.item.materialType = m.materialType();
            data.item.size = m.size();
            data.item.laborClass = m.laborClass();
            groups[key] = data;
        }
        
        GroupData& data = groups[key];
        data.item.totalLengthInches += m.lengthInches();
        data.item.itemCount++;
        
        // Track length by shape for weight calculation
        if (m.shapeId() >= 0) {
            double lengthFt = m.lengthInches() / 12.0;
            data.shapeIdLengths[m.shapeId()] += lengthFt;
        }
    }
    
    // Calculate costs, weights and build line items
    double totalMaterial = 0.0;
    double totalLabor = 0.0;
    double totalWeight = 0.0;
    
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        GroupData& data = it.value();
        QuoteLineItem& item = data.item;
        
        // Convert to feet
        item.totalLengthFeet = item.totalLengthInches / 12.0;
        
        // Calculate weight from shapes database
        if (shapesDb && shapesDb->isOpen()) {
            for (auto shapeIt = data.shapeIdLengths.begin(); 
                 shapeIt != data.shapeIdLengths.end(); ++shapeIt) {
                int shapeId = shapeIt.key();
                double lengthFt = shapeIt.value();
                double weightPerFt = shapesDb->getShapeProperty(shapeId, "W");
                item.totalWeightLb += lengthFt * weightPerFt;
            }
        }
        totalWeight += item.totalWeightLb;
        
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
    summary.grandTotalWeight = totalWeight;
    
    return summary;
}

QString QuoteCalculator::groupKey(MaterialType material, const QString& size, LaborClass labor) const
{
    return QString("%1|%2|%3")
        .arg(static_cast<int>(material))
        .arg(size)
        .arg(static_cast<int>(labor));
}

