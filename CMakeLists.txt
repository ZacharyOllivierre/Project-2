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
    src/database/database.cpp
    src/database/database.h
    src/mainwindow/mainwindow.cpp
    src/mainwindow/mainwindow.h
    src/teaminfowidget/teaminfowidget.cpp
    src/teaminfowidget/teaminfowidget.h
    src/souvenir/souvenirmanager.cpp
    src/souvenir/souvenirmanager.h
    src/purchase/purchasewindow.cpp
    src/purchase/purchasewindow.h
    src/browse/browsewidget.h
    src/browse/browsewidget.cpp
    src/browse/browsewidget.ui
    src/graph/graphvisualizer.h
    src/graph/graphvisualizer.cpp
    src/admin/adminwidget.h       # <-- NEW
    src/admin/adminwidget.cpp     # <-- NEW
    src/astar/astar.cpp
    src/astar/astar.h
    src/homepage/homepage.cpp
    src/homepage/homepage.h
    src/homepage/homepage.ui
)
target_link_libraries(Project2 PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::Core
)
