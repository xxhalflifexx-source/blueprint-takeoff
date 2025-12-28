#include "BlueprintView.h"
#include "MathUtils.h"

#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QInputDialog>
#include <QGraphicsEllipseItem>
#include <QPen>
#include <QBrush>
#include <QScrollBar>
#include <cmath>

// Color constants
const QColor BlueprintView::TEMP_COLOR(255, 165, 0);        // Orange for temporary
const QColor BlueprintView::MEASUREMENT_COLOR(0, 150, 0);   // Green for completed
const QColor BlueprintView::HIGHLIGHT_COLOR(255, 0, 0);     // Red for highlighted
const QColor BlueprintView::POINT_COLOR(0, 100, 255);       // Blue for points

BlueprintView::BlueprintView(QWidget* parent)
    : QGraphicsView(parent)
    , m_scene(nullptr)
    , m_imageItem(nullptr)
    , m_currentTool(Tool::None)
    , m_tempStartPoint(nullptr)
    , m_rubberBand(nullptr)
    , m_highlightedMeasurementId(-1)
    , m_isPanning(false)
    , m_nextMeasurementId(1)
{
    setupScene();
    setMouseTracking(true);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::NoDrag);
    setFocusPolicy(Qt::StrongFocus);
}

BlueprintView::~BlueprintView()
{
    // Scene cleanup is automatic with parent
}

void BlueprintView::setupScene()
{
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    m_scene->setBackgroundBrush(QBrush(QColor(255, 255, 255)));  // White background
}

bool BlueprintView::loadImage(const QString& filePath)
{
    QPixmap pixmap(filePath);
    if (pixmap.isNull()) {
        return false;
    }

    displayPixmap(pixmap);
    return true;
}

bool BlueprintView::loadFromImage(const QImage& image)
{
    if (image.isNull()) {
        return false;
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.isNull()) {
        return false;
    }

    displayPixmap(pixmap);
    return true;
}

void BlueprintView::displayPixmap(const QPixmap& pixmap)
{
    // Clear existing content
    m_scene->clear();
    m_measurementGraphics.clear();
    m_tempLines.clear();
    m_tempStartPoint = nullptr;
    m_rubberBand = nullptr;
    m_tempPoints.clear();

    // Add the image
    m_imageItem = m_scene->addPixmap(pixmap);
    m_imageItem->setZValue(0);  // Image at bottom
    m_imageItem->setTransformationMode(Qt::SmoothTransformation);  // High quality when zoomed

    // Fit the view to the image
    m_scene->setSceneRect(pixmap.rect());
    fitInView(m_imageItem, Qt::KeepAspectRatio);
}

bool BlueprintView::hasImage() const
{
    return m_imageItem != nullptr;
}

void BlueprintView::clearImage()
{
    m_scene->clear();
    m_imageItem = nullptr;
    m_measurementGraphics.clear();
    m_tempLines.clear();
    m_tempStartPoint = nullptr;
    m_rubberBand = nullptr;
    m_tempPoints.clear();
    m_calibration.reset();
    m_nextMeasurementId = 1;
}

void BlueprintView::setTool(Tool tool)
{
    if (m_currentTool != Tool::None) {
        cancelCurrentTool();
    }
    m_currentTool = tool;
    
    // Update cursor
    if (tool == Tool::None) {
        setCursor(Qt::OpenHandCursor);
    } else {
        setCursor(Qt::CrossCursor);
    }
}

Tool BlueprintView::currentTool() const
{
    return m_currentTool;
}

const Calibration& BlueprintView::calibration() const
{
    return m_calibration;
}

Calibration& BlueprintView::calibration()
{
    return m_calibration;
}

void BlueprintView::setCalibration(const Calibration& calibration)
{
    m_calibration = calibration;
}

void BlueprintView::addMeasurement(const Measurement& measurement)
{
    createMeasurementGraphics(measurement);
}

void BlueprintView::removeMeasurement(int measurementId)
{
    if (m_measurementGraphics.contains(measurementId)) {
        // Remove graphics items
        for (QGraphicsItem* item : m_measurementGraphics[measurementId]) {
            m_scene->removeItem(item);
            delete item;
        }
        m_measurementGraphics.remove(measurementId);
        
        // Clear highlight if this was the highlighted measurement
        if (m_highlightedMeasurementId == measurementId) {
            m_highlightedMeasurementId = -1;
        }
    }
}

void BlueprintView::highlightMeasurement(int measurementId)
{
    // Restore previous highlighted measurement to normal color
    if (m_highlightedMeasurementId >= 0 && m_measurementGraphics.contains(m_highlightedMeasurementId)) {
        for (QGraphicsItem* item : m_measurementGraphics[m_highlightedMeasurementId]) {
            if (auto* lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
                QPen pen = lineItem->pen();
                pen.setColor(MEASUREMENT_COLOR);
                lineItem->setPen(pen);
            } else if (auto* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                ellipseItem->setBrush(QBrush(MEASUREMENT_COLOR));
            }
        }
    }

    m_highlightedMeasurementId = measurementId;

    // Highlight the new measurement
    if (measurementId >= 0 && m_measurementGraphics.contains(measurementId)) {
        for (QGraphicsItem* item : m_measurementGraphics[measurementId]) {
            if (auto* lineItem = dynamic_cast<QGraphicsLineItem*>(item)) {
                QPen pen = lineItem->pen();
                pen.setColor(HIGHLIGHT_COLOR);
                lineItem->setPen(pen);
            } else if (auto* ellipseItem = dynamic_cast<QGraphicsEllipseItem*>(item)) {
                ellipseItem->setBrush(QBrush(HIGHLIGHT_COLOR));
            }
        }
    }
}

void BlueprintView::clearMeasurements()
{
    for (auto it = m_measurementGraphics.begin(); it != m_measurementGraphics.end(); ++it) {
        for (QGraphicsItem* item : it.value()) {
            m_scene->removeItem(item);
            delete item;
        }
    }
    m_measurementGraphics.clear();
    m_highlightedMeasurementId = -1;
}

void BlueprintView::setNextMeasurementId(int nextId)
{
    m_nextMeasurementId = nextId;
}

void BlueprintView::wheelEvent(QWheelEvent* event)
{
    // Zoom in/out with mouse wheel
    const double zoomFactor = 1.15;
    
    if (event->angleDelta().y() > 0) {
        scale(zoomFactor, zoomFactor);
    } else {
        scale(1.0 / zoomFactor, 1.0 / zoomFactor);
    }
    
    event->accept();
}

void BlueprintView::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton || 
        (event->button() == Qt::LeftButton && m_currentTool == Tool::None)) {
        // Start panning
        m_isPanning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && m_currentTool != Tool::None) {
        QPointF scenePos = mapToScene(event->pos());
        
        // Add point to temporary points
        m_tempPoints.append(scenePos);

        // Create visual point marker
        if (m_tempPoints.size() == 1) {
            // First point - create start marker
            const double pointRadius = 4.0;
            m_tempStartPoint = m_scene->addEllipse(
                scenePos.x() - pointRadius, scenePos.y() - pointRadius,
                pointRadius * 2, pointRadius * 2,
                QPen(TEMP_COLOR, 2),
                QBrush(POINT_COLOR)
            );
            m_tempStartPoint->setZValue(100);
        }

        // For line tool, check if we have 2 points
        if (m_currentTool == Tool::Line && m_tempPoints.size() == 2) {
            finishLineMeasurement();
        }
        // For calibration, check if we have 2 points
        else if (m_currentTool == Tool::Calibrate && m_tempPoints.size() == 2) {
            finishCalibration();
        }
        // For polyline, add line segment
        else if (m_currentTool == Tool::Polyline && m_tempPoints.size() >= 2) {
            // Add a permanent temp line for this segment
            QPointF p1 = m_tempPoints[m_tempPoints.size() - 2];
            QPointF p2 = m_tempPoints[m_tempPoints.size() - 1];
            QGraphicsLineItem* line = m_scene->addLine(
                p1.x(), p1.y(), p2.x(), p2.y(),
                QPen(TEMP_COLOR, 2)
            );
            line->setZValue(99);
            m_tempLines.append(line);
        }

        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void BlueprintView::mouseMoveEvent(QMouseEvent* event)
{
    if (m_isPanning) {
        // Pan the view
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();
        
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        
        event->accept();
        return;
    }

    // Update rubber band line if we have at least one point
    if (m_currentTool != Tool::None && !m_tempPoints.isEmpty()) {
        QPointF scenePos = mapToScene(event->pos());
        updateTempDrawing(scenePos);
        
        // Emit live measurement
        double lengthPixels = calculateCurrentLength();
        if (m_tempPoints.size() >= 1) {
            lengthPixels += MathUtils::distance(m_tempPoints.last(), scenePos);
        }
        double lengthInches = lengthPixels / m_calibration.pixelsPerInch();
        emit liveMeasurementChanged(lengthInches);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void BlueprintView::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_isPanning && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
        m_isPanning = false;
        setCursor(m_currentTool == Tool::None ? Qt::OpenHandCursor : Qt::CrossCursor);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void BlueprintView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_currentTool == Tool::Polyline) {
        // Finish polyline on double-click (need at least 2 points)
        if (m_tempPoints.size() >= 2) {
            finishPolylineMeasurement();
        }
        event->accept();
        return;
    }

    QGraphicsView::mouseDoubleClickEvent(event);
}

void BlueprintView::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        cancelCurrentTool();
        emit toolCancelled();
        event->accept();
        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void BlueprintView::cancelCurrentTool()
{
    clearTempDrawing();
    m_tempPoints.clear();
    emit liveMeasurementChanged(0.0);
}

void BlueprintView::updateTempDrawing(const QPointF& currentPos)
{
    if (m_tempPoints.isEmpty()) {
        return;
    }

    // Update or create rubber band line from last point to mouse
    QPointF lastPoint = m_tempPoints.last();
    
    if (!m_rubberBand) {
        m_rubberBand = m_scene->addLine(
            lastPoint.x(), lastPoint.y(),
            currentPos.x(), currentPos.y(),
            QPen(TEMP_COLOR, 2, Qt::DashLine)
        );
        m_rubberBand->setZValue(100);
    } else {
        m_rubberBand->setLine(lastPoint.x(), lastPoint.y(), currentPos.x(), currentPos.y());
    }
}

void BlueprintView::clearTempDrawing()
{
    // Remove rubber band
    if (m_rubberBand) {
        m_scene->removeItem(m_rubberBand);
        delete m_rubberBand;
        m_rubberBand = nullptr;
    }

    // Remove start point marker
    if (m_tempStartPoint) {
        m_scene->removeItem(m_tempStartPoint);
        delete m_tempStartPoint;
        m_tempStartPoint = nullptr;
    }

    // Remove temp lines
    for (QGraphicsLineItem* line : m_tempLines) {
        m_scene->removeItem(line);
        delete line;
    }
    m_tempLines.clear();
}

void BlueprintView::finishCalibration()
{
    if (m_tempPoints.size() != 2) {
        return;
    }

    // Ask user for the real-world distance
    bool ok;
    double realDistance = QInputDialog::getDouble(
        this,
        "Calibration",
        "Enter the real-world distance between the two points (in inches):",
        12.0,   // Default value
        0.01,   // Min
        100000, // Max
        2,      // Decimals
        &ok
    );

    if (ok && realDistance > 0) {
        m_calibration.calibrate(m_tempPoints[0], m_tempPoints[1], realDistance);
        emit calibrationCompleted(m_calibration.pixelsPerInch());
    }

    // Clean up
    clearTempDrawing();
    m_tempPoints.clear();
    setTool(Tool::None);
}

void BlueprintView::finishLineMeasurement()
{
    if (m_tempPoints.size() != 2) {
        return;
    }

    double pixelLength = MathUtils::distance(m_tempPoints[0], m_tempPoints[1]);
    double inchLength = pixelLength / m_calibration.pixelsPerInch();

    Measurement measurement(m_nextMeasurementId++, MeasurementType::Line, m_tempPoints, inchLength);
    
    // Clean up temp drawing (MainWindow will create permanent graphics)
    clearTempDrawing();
    m_tempPoints.clear();

    emit measurementCompleted(measurement);
    emit liveMeasurementChanged(0.0);
}

void BlueprintView::finishPolylineMeasurement()
{
    if (m_tempPoints.size() < 2) {
        return;
    }

    double pixelLength = MathUtils::polylineLength(m_tempPoints);
    double inchLength = pixelLength / m_calibration.pixelsPerInch();

    Measurement measurement(m_nextMeasurementId++, MeasurementType::Polyline, m_tempPoints, inchLength);
    
    // Clean up temp drawing (MainWindow will create permanent graphics)
    clearTempDrawing();
    m_tempPoints.clear();

    emit measurementCompleted(measurement);
    emit liveMeasurementChanged(0.0);
}

void BlueprintView::createMeasurementGraphics(const Measurement& measurement)
{
    QVector<QGraphicsItem*> items;
    const QVector<QPointF>& points = measurement.points();
    
    QPen linePen(MEASUREMENT_COLOR, 2);
    const double pointRadius = 3.0;

    // Draw lines between points
    for (int i = 1; i < points.size(); ++i) {
        QGraphicsLineItem* line = m_scene->addLine(
            points[i-1].x(), points[i-1].y(),
            points[i].x(), points[i].y(),
            linePen
        );
        line->setZValue(50);
        items.append(line);
    }

    // Draw point markers
    for (const QPointF& pt : points) {
        QGraphicsEllipseItem* ellipse = m_scene->addEllipse(
            pt.x() - pointRadius, pt.y() - pointRadius,
            pointRadius * 2, pointRadius * 2,
            QPen(Qt::black, 1),
            QBrush(MEASUREMENT_COLOR)
        );
        ellipse->setZValue(51);
        items.append(ellipse);
    }

    m_measurementGraphics[measurement.id()] = items;
}

double BlueprintView::calculateCurrentLength() const
{
    return MathUtils::polylineLength(m_tempPoints);
}
