#-------------------------------------------------
#
# Project created by QtCreator 2015-02-06T19:05:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DACUSN
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    reciever.cpp \
    rawdata.cpp \
    uwbsettings.cpp \
    datainputthreadworker.cpp \
    stackmanager.cpp \
    radarunit.cpp \
    datainputdialog.cpp \
    stackmanagerdialog.cpp

HEADERS  += mainwindow.h \
    reciever.h \
    rawdata.h \
    stddefs.h \
    uwbsettings.h \
    datainputthreadworker.h \
    stackmanager.h \
    radarunit.h \
    radar_handler.h \
    datainputdialog.h \
    stackmanagerdialog.h

FORMS    += mainwindow.ui \
    datainputdialog.ui \
    stackmanagerdialog.ui

RESOURCES += \
    icons.qrc
