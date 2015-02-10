#-------------------------------------------------
#
# Project created by QtCreator 2015-02-06T19:05:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ADCUSN
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    reciever.cpp \
    rawdata.cpp \
    datainputthread.cpp \
    uwbsettings.cpp

HEADERS  += mainwindow.h \
    reciever.h \
    rawdata.h \
    stddefs.h \
    datainputthread.h \
    uwbsettings.h

FORMS    += mainwindow.ui
