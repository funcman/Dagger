#-------------------------------------------------
#
# Project created by QtCreator 2015-07-01T15:32:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Client
TEMPLATE = app

INCLUDEPATH = ../Base ../Engine

QMAKE_LIBDIR += ../Lib

DESTDIR = ../Bin

LIBS = -lBase -lEngine

SOURCES +=              \
    Main.cpp            \
    MainWindow.cpp      \
    MainTask.cpp        \
    ScreenWidget.cpp

HEADERS  +=             \
    MainWindow.h        \
    MainTask.h          \
    ScreenWidget.h

FORMS    +=             \
    MainWindow.ui
