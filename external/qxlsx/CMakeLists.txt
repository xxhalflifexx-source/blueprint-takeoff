# QXlsx library wrapper using FetchContent
# This downloads QXlsx from GitHub at configure time

include(FetchContent)

FetchContent_Declare(
    QXlsx
    GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
    GIT_TAG        v1.4.9
)

FetchContent_MakeAvailable(QXlsx)

