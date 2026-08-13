cmake_minimum_required(VERSION 3.16)
project(ParquetPlot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport REQUIRED)
find_package(Arrow REQUIRED)
find_package(Parquet REQUIRED)

# QCustomPlot is not consistently packaged by Linux distributions and is not
# normally present on Windows. Download a pinned, checksum-verified source
# release and compile it into ParquetPlot so end users do not need to install it.
option(PARQUETPLOT_USE_SYSTEM_QCUSTOMPLOT
       "Use an installed qcustomplot-qt6 library instead of the pinned source" OFF)

if(PARQUETPLOT_USE_SYSTEM_QCUSTOMPLOT)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(QCUSTOMPLOT QUIET IMPORTED_TARGET qcustomplot-qt6)
    endif()

    if(QCUSTOMPLOT_FOUND)
        set(QCUSTOMPLOT_TARGET PkgConfig::QCUSTOMPLOT)
    else()
        find_path(QCUSTOMPLOT_INCLUDE_DIR qcustomplot.h)
        find_library(QCUSTOMPLOT_LIBRARY NAMES qcustomplot-qt6 qcustomplot)
        if(NOT QCUSTOMPLOT_INCLUDE_DIR OR NOT QCUSTOMPLOT_LIBRARY)
            message(FATAL_ERROR
                "QCustomPlot was not found. Disable PARQUETPLOT_USE_SYSTEM_QCUSTOMPLOT "
                "to use the bundled-at-build source release.")
        endif()

        add_library(qcustomplot_system UNKNOWN IMPORTED)
        set_target_properties(qcustomplot_system PROPERTIES
            IMPORTED_LOCATION "${QCUSTOMPLOT_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${QCUSTOMPLOT_INCLUDE_DIR}")
        set(QCUSTOMPLOT_TARGET qcustomplot_system)
    endif()
else()
    include(FetchContent)
    if(POLICY CMP0135)
        cmake_policy(SET CMP0135 NEW)
    endif()
    FetchContent_Declare(qcustomplot
        URL https://www.qcustomplot.com/release/2.1.1/QCustomPlot.tar.gz
        URL_HASH SHA512=2e49a9b3f7ab03bcd580e5f3c3ae0d5e8c57d3ccce0ceed9862cde7ea23e2f3672a963af988be60e504cb5aa50bc462e4b28acf577eae41cc6fea2802642dc19)
    FetchContent_MakeAvailable(qcustomplot)

    add_library(qcustomplot_source STATIC
        "${qcustomplot_SOURCE_DIR}/qcustomplot.cpp"
        "${qcustomplot_SOURCE_DIR}/qcustomplot.h")
    target_include_directories(qcustomplot_source PUBLIC "${qcustomplot_SOURCE_DIR}")
    target_link_libraries(qcustomplot_source PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::PrintSupport)
    set(QCUSTOMPLOT_TARGET qcustomplot_source)
endif()

add_executable(${PROJECT_NAME} WIN32
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/parquet_reader.cpp
    src/parquet_reader.h)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Arrow::arrow_shared
    Parquet::parquet_shared
    ${QCUSTOMPLOT_TARGET})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES resources/io.github.selimct.ParquetPlot.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES resources/io.github.selimct.ParquetPlot.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES LICENSE THIRD_PARTY_NOTICES.md
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/ParquetPlot)
