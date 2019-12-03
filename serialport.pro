#-------------------------------------------------
#
# Project created by QtCreator 2016-05-22T13:08:09
#
#-------------------------------------------------

QT       += serialport core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = serialport
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    define_protocol.h

FORMS    += mainwindow.ui
