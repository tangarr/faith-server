#-------------------------------------------------
#
# Project created by QtCreator 2014-04-23T16:15:46
#
#-------------------------------------------------

QT       -= gui
QT       += network

TARGET = faith-core
TEMPLATE = lib
CONFIG += staticlib

SOURCES += faithcore.cpp \
    faithmessage.cpp \
    faithdatabuilder.cpp \
    fdstring.cpp \
    fdstringlist.cpp \
    fdproposedip.cpp

HEADERS += faithcore.h \
    faithmessage.h \
    faithdata.h \
    faithdatabuilder.h \
    fdstring.h \
    fdstringlist.h \
    fdproposedip.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
