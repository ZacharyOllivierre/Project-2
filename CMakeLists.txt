cmake_minimum_required(VERSION 3.16)

project(Project2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core)

qt_add_executable(Project2
    main.cpp
    database.cpp
    database.h
    mainwindow.cpp
    mainwindow.h
    teaminfowidget.cpp
    teaminfowidget.h
    souvenirmanager.cpp
    souvenirmanager.h
    purchasewindow.cpp
    purchasewindow.h
)

target_link_libraries(Project2 PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Core
)
