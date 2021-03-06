#-------------------------------------------------
#
# Project created by QtCreator 2015-02-06T19:05:14
#
#-------------------------------------------------

QT       += core gui opengl serialport

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
    stackmanagerdialog.cpp \
    radarlistdialog.cpp \
    visualization.cpp \
    scenerendererdialog.cpp \
    coordinatesinputdialog.cpp \
    radarsubwindow.cpp \
    backupoptionsdialog.cpp \
    uwbpacketclass.cpp \
    rs232.c \
    mttsettingsdialog.cpp \
    mtt_pure.cpp

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
    stackmanagerdialog.h \
    radarlistdialog.h \
    visualization.h \
    scenerendererdialog.h \
    coordinatesinputdialog.h \
    radarsubwindow.h \
    backupoptionsdialog.h \
    rs232.h \
    uwbpacketclass.h \
    mttsettingsdialog.h \
    mtt_pure.h

FORMS    += mainwindow.ui \
    datainputdialog.ui \
    stackmanagerdialog.ui \
    radarlistdialog.ui \
    scenerendererdialog.ui \
    coordinatesinputdialog.ui \
    radarsubwindow.ui \
    backupoptionsdialog.ui \
    mttsettingsdialog.ui

RESOURCES += \
    icons.qrc

CONFIG += qwt

RC_FILE = iconrc.rc
