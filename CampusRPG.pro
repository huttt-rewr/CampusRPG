QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CampusRPG_GUI
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/MapWidget.cpp \
    src/Character.cpp \
    src/Item.cpp \
    src/Food.cpp \
    src/Medicine.cpp \
    src/Equipment.cpp \
    src/Enemy.cpp \
    src/Task.cpp \
    src/Shop.cpp \
    src/GameManager.cpp \
    src/Location.cpp

HEADERS += \
    src/MainWindow.h \
    src/MapWidget.h \
    include/Character.h \
    include/Item.h \
    include/Food.h \
    include/Medicine.h \
    include/Equipment.h \
    include/Enemy.h \
    include/Task.h \
    include/Shop.h \
    include/GameManager.h \
    include/Location.h

INCLUDEPATH += include src
