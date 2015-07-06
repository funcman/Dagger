#-------------------------------------------------
#
# Project created by QtCreator 2015-07-01T15:34:37
#
#-------------------------------------------------

QT       -= core gui

TARGET = Engine
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH = ../Base

DESTDIR = ../Lib

SOURCES +=          \
    Color.cpp       \
    DrawPixel.cpp   \
    DrawLine.cpp    \
    Canvas.cpp

HEADERS +=          \
    Engine.h        \
    Macros.h        \
    Color.h         \
    DrawPixel.h     \
    DrawLine.h      \
    Canvas.h
