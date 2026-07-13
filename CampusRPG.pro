QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CampusRPG_GUI
TEMPLATE = app
CONFIG += c++17

SOURCES += \
    src/main.cpp \
    src/MainWindow.cpp \
    src/Profession.cpp

HEADERS += \
    src/MainWindow.h \
    include/Profession.h

INCLUDEPATH += include src

RESOURCES += resources.qrc
