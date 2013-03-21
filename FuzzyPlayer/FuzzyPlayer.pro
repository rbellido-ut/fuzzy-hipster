#-------------------------------------------------
#
# Project created by QtCreator 2013-03-20T17:32:39
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FuzzyPlayer
TEMPLATE = app


SOURCES += main.cpp\
        fuzzymainwindow.cpp \
    client.cpp \
    communication.cpp \
    util.cpp

HEADERS  += fuzzymainwindow.h \
    client.h \
    communication.h \
    util.h

FORMS    += fuzzymainwindow.ui
