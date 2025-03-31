cmake_minimum_required(VERSION 3.16)

project(LaserSpectraVue VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)





# Qt-specific settings
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Find Qt packages
find_package(Qt6 COMPONENTS Core Gui Widgets Charts OpenGL Svg REQUIRED)

# Setup windeployqt
get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

# FTD2XX library setup
set(FTD2XX_DIR "E:/cameracodes/test1/test1/CDM/amd64")
set(FTD2XX_LIBRARY_PATH "${FTD2XX_DIR}/ftd2xx.lib")

if(NOT EXISTS ${FTD2XX_LIBRARY_PATH})
    message(FATAL_ERROR "FTD2XX library not found at ${FTD2XX_LIBRARY_PATH}")
endif()

# Project sources
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        resources.qrc
        appicon.rc
)

# Create executable
qt_add_executable(LaserSpectraVue
        ${PROJECT_SOURCES}
)

# Set include directories
target_include_directories(LaserSpectraVue PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${FTD2XX_DIR}
)

# Link libraries
target_link_libraries(LaserSpectraVue PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Charts
        Qt6::OpenGL
        Qt6::Svg
        ${FTD2XX_LIBRARY_PATH}
)

# Set application properties
set_target_properties(LaserSpectraVue PROPERTIES
        WIN32_EXECUTABLE TRUE
)

# Installation
install(TARGETS LaserSpectraVue
        RUNTIME DESTINATION bin
)

# Add custom command to run windeployqt and copy FTD2XX DLL
add_custom_command(TARGET LaserSpectraVue POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:LaserSpectraVue>"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${FTD2XX_DIR}/ftd2xx.dll" "$<TARGET_FILE_DIR:LaserSpectraVue>"
        COMMENT "Running windeployqt and copying FTD2XX DLL..."
)