#include <QApplication>
#include "MainWindow.h"

/**
 * @brief Entry point for the Blueprint Takeoff MVP application.
 * 
 * A simple desktop application for measuring distances on blueprint images.
 * 
 * Features:
 * - Open PNG/JPG blueprint images
 * - Pan and zoom navigation
 * - Calibration tool to set pixels-per-inch scale
 * - Line measurement tool
 * - Polyline measurement tool
 * - Measurement list with selection/highlighting
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application metadata
    app.setApplicationName("Blueprint Takeoff");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Blueprint Tools");
    
    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}

