cmake_minimum_required(VERSION 3.16)
project(Project2 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Core)
qt_add_executable(Project2
    src/main.cpp
    src/database.cpp
    src/database.h
    src/mainwindow.cpp
    src/mainwindow.h
    src/teaminfowidget.cpp
    src/teaminfowidget.h
    src/souvenirmanager.cpp
    src/souvenirmanager.h
    src/purchasewindow.cpp
    src/purchasewindow.h
    src/browsewidget.h src/browsewidget.cpp src/browsewidget.ui
    src/graphvisualizer.h src/graphvisualizer.cpp
)
target_link_libraries(Project2 PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Core
)
