cmake_minimum_required(VERSION 3.16)

project(Project2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core)

include_directories(${CMAKE_SOURCE_DIR}/src)

qt_add_executable(Project2
    ${CMAKE_SOURCE_DIR}/src/main.cpp

    ${CMAKE_SOURCE_DIR}/src/database.cpp
    ${CMAKE_SOURCE_DIR}/src/database.h

    ${CMAKE_SOURCE_DIR}/src/mainwindow.cpp
    ${CMAKE_SOURCE_DIR}/src/mainwindow.h

    ${CMAKE_SOURCE_DIR}/src/teaminfowidget.cpp
    ${CMAKE_SOURCE_DIR}/src/teaminfowidget.h

    ${CMAKE_SOURCE_DIR}/src/souvenirmanager.cpp
    ${CMAKE_SOURCE_DIR}/src/souvenirmanager.h

    ${CMAKE_SOURCE_DIR}/src/purchasewindow.cpp
    ${CMAKE_SOURCE_DIR}/src/purchasewindow.h

    ${CMAKE_SOURCE_DIR}/src/stadiumgraph.cpp
    ${CMAKE_SOURCE_DIR}/src/stadiumgraph.h

    ${CMAKE_SOURCE_DIR}/src/browsewidget.cpp
    ${CMAKE_SOURCE_DIR}/src/browsewidget.h
    ${CMAKE_SOURCE_DIR}/src/browsewidget.ui

    ${CMAKE_SOURCE_DIR}/src/adminwidget.cpp
    ${CMAKE_SOURCE_DIR}/src/adminwidget.h

    ${CMAKE_SOURCE_DIR}/src/homepage.cpp
    ${CMAKE_SOURCE_DIR}/src/homepage.h
    ${CMAKE_SOURCE_DIR}/src/homepage.ui
)

target_link_libraries(Project2 PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Core
)