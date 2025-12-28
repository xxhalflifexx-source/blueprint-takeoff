cmake_minimum_required(VERSION 3.16)
project(BlueprintTakeoff VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt6 settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 Widgets and Sql (required)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

# Find Qt6 Pdf (optional)
find_package(Qt6 COMPONENTS Pdf QUIET)
if(Qt6Pdf_FOUND)
    message(STATUS "Qt PDF module found - PDF support enabled")
    add_compile_definitions(HAS_QT_PDF)
else()
    message(STATUS "Qt PDF module not found - PDF support disabled (images only)")
endif()

# Source files
set(CORE_SOURCES
    src/core/MathUtils.cpp
    src/core/CoordinateTransform.cpp
    src/core/PdfRenderer.cpp
    src/core/ProjectDatabase.cpp
)

set(CORE_HEADERS
    src/core/MathUtils.h
    src/core/CoordinateTransform.h
    src/core/PdfRenderer.h
    src/core/ProjectDatabase.h
)

set(MODEL_SOURCES
    src/models/Measurement.cpp
    src/models/TakeoffItem.cpp
    src/models/Calibration.cpp
    src/models/Project.cpp
    src/models/Page.cpp
)

set(MODEL_HEADERS
    src/models/Measurement.h
    src/models/TakeoffItem.h
    src/models/Calibration.h
    src/models/Project.h
    src/models/Page.h
)

set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/BlueprintView.cpp
    src/ui/MeasurementPanel.cpp
    src/ui/PagesPanel.cpp
    src/ui/UndoCommands.cpp
    src/ui/PropertiesDock.cpp
    src/ui/QuoteDock.cpp
    src/ui/PdfImportDialog.cpp
    src/ui/ShapePickerDialog.cpp
)

set(UI_HEADERS
    src/ui/MainWindow.h
    src/ui/BlueprintView.h
    src/ui/MeasurementPanel.h
    src/ui/PagesPanel.h
    src/ui/UndoCommands.h
    src/ui/PropertiesDock.h
    src/ui/QuoteDock.h
    src/ui/PdfImportDialog.h
    src/ui/ShapePickerDialog.h
)

# Create executable
add_executable(BlueprintTakeoff
    src/main.cpp
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${MODEL_SOURCES}
    ${MODEL_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
)

# Include directories
target_include_directories(BlueprintTakeoff PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/ui
)

# Link Qt6 Widgets and Sql (and Pdf if available)
target_link_libraries(BlueprintTakeoff PRIVATE Qt6::Widgets Qt6::Sql)
if(Qt6Pdf_FOUND)
    target_link_libraries(BlueprintTakeoff PRIVATE Qt6::Pdf)
endif()

# Windows-specific: Set as GUI application (no console window)
if(WIN32)
    set_target_properties(BlueprintTakeoff PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
