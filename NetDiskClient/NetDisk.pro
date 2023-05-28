#-------------------------------------------------
#
# Project created by QtCreator 2023-01-07T11:55:12
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetDisk
TEMPLATE = app

RC_ICONS = ./images/logo.ico


SOURCES += main.cpp\
        maindialog.cpp \
    ckernel.cpp \
    logindialog.cpp \
    mytablewightitem.cpp

HEADERS  += maindialog.h \
    ckernel.h \
    logindialog.h \
    common.h \
    mytablewightitem.h

FORMS    += maindialog.ui \
    logindialog.ui


include(./netapi/netapi.pri)
INCLUDEPATH += ./netapi/net
INCLUDEPATH += ./netapi/mediator


include(./md5/md5.pri)
INCLUDEPATH +=./md5

include(./sqlapi/sqlapi.pri)
INCLUDEPATH += ./sqlapi

RESOURCES += \
    resource.qrc
