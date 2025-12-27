#ifndef BLUEPRINTVIEW_H
#define BLUEPRINTVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QVector>
#include <QPointF>
#include <QMap>
#include <QImage>

#include "Measurement.h"
#include "Calibration.h"

/**
 * @brief Active tool mode for the blueprint view.
 */
enum class Tool
{
    None,       // No tool active, pan mode
    Calibrate,  // Calibration mode
    Line,       // Line measurement tool
    Polyline    // Polyline measurement tool
};

/**
 * @brief Graphics view for displaying and interacting with blueprint images.
 * 
 * Provides pan/zoom functionality and tools for calibration and measurement.
 * Supports both direct image files and rendered QImages (e.g., from PDF).
 */
class BlueprintView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit BlueprintView(QWidget* parent = nullptr);
    ~BlueprintView();

    /**
     * @brief Load and display a blueprint image from file.
     * @param filePath Path to the image file
     * @return true if loaded successfully
     */
    bool loadImage(const QString& filePath);

    /**
     * @brief Load and display a blueprint from a QImage.
     * @param image The image to display
     * @return true if loaded successfully
     * 
     * Used for displaying rendered PDF pages.
     */
    bool loadFromImage(const QImage& image);

    /**
     * @brief Check if an image is currently loaded.
     * @return true if an image is displayed
     */
    bool hasImage() const;

    /**
     * @brief Clear the current image from the view.
     */
    void clearImage();

    /**
     * @brief Set the active tool.
     * @param tool Tool to activate
     */
    void setTool(Tool tool);

    /**
     * @brief Get the current active tool.
     * @return Current tool
     */
    Tool currentTool() const;

    /**
     * @brief Get the calibration data (const).
     * @return Reference to calibration object
     */
    const Calibration& calibration() const;

    /**
     * @brief Get the calibration data (mutable).
     * @return Reference to calibration object
     */
    Calibration& calibration();

    /**
     * @brief Set the calibration data.
     * @param calibration Calibration to apply
     */
    void setCalibration(const Calibration& calibration);

    /**
     * @brief Add a completed measurement to display.
     * @param measurement The measurement to add
     */
    void addMeasurement(const Measurement& measurement);

    /**
     * @brief Remove a measurement from display.
     * @param measurementId ID of measurement to remove
     */
    void removeMeasurement(int measurementId);

    /**
     * @brief Highlight a specific measurement.
     * @param measurementId ID of measurement to highlight, -1 to clear
     */
    void highlightMeasurement(int measurementId);

    /**
     * @brief Clear all measurements from the view.
     */
    void clearMeasurements();

    /**
     * @brief Reset the next measurement ID counter.
     * @param nextId The next ID to use
     */
    void setNextMeasurementId(int nextId);

signals:
    /**
     * @brief Emitted when calibration is completed.
     * @param pixelsPerInch The calculated scale factor
     */
    void calibrationCompleted(double pixelsPerInch);

    /**
     * @brief Emitted when a measurement is completed.
     * @param measurement The completed measurement
     */
    void measurementCompleted(const Measurement& measurement);

    /**
     * @brief Emitted when live measurement value changes during drawing.
     * @param inches Current measured distance in inches
     */
    void liveMeasurementChanged(double inches);

    /**
     * @brief Emitted when tool operation is cancelled.
     */
    void toolCancelled();

protected:
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupScene();
    void cancelCurrentTool();
    void updateTempDrawing(const QPointF& currentPos);
    void clearTempDrawing();
    void finishCalibration();
    void finishLineMeasurement();
    void finishPolylineMeasurement();
    void createMeasurementGraphics(const Measurement& measurement);
    double calculateCurrentLength() const;
    void displayPixmap(const QPixmap& pixmap);

    // Scene and image
    QGraphicsScene* m_scene;
    QGraphicsPixmapItem* m_imageItem;

    // Tool state
    Tool m_currentTool;
    QVector<QPointF> m_tempPoints;
    
    // Calibration
    Calibration m_calibration;

    // Temporary drawing items (during tool use)
    QVector<QGraphicsLineItem*> m_tempLines;
    QGraphicsEllipseItem* m_tempStartPoint;
    QGraphicsLineItem* m_rubberBand;

    // Permanent measurement graphics
    // Map from measurement ID to list of graphics items
    QMap<int, QVector<QGraphicsItem*>> m_measurementGraphics;
    int m_highlightedMeasurementId;

    // Pan state
    bool m_isPanning;
    QPoint m_lastPanPoint;

    // Measurement ID counter
    int m_nextMeasurementId;

    // Colors
    static const QColor TEMP_COLOR;
    static const QColor MEASUREMENT_COLOR;
    static const QColor HIGHLIGHT_COLOR;
    static const QColor POINT_COLOR;
};

#endif // BLUEPRINTVIEW_H
