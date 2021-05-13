#-------------------------------------------------
#
# Project created by QtCreator 2021-01-17T22:32:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include (./netapi/netapi.pri)
INCLUDEPATH += $$PWD/netapi

include (./RecordVideo/RecordVideo.pri)
INCLUDEPATH += $$PWD/RecordVideo

include (./RecordAudio/RecordAudio.pri)
INCLUDEPATH += $$PWD/RecordAudio

TARGET = WeTalk
TEMPLATE = app


SOURCES += main.cpp\
        wetalk.cpp \
    logindialog.cpp \
    chatdialog.cpp \
    useritem.cpp \
    IMToolBox.cpp \
    addfriend.cpp \
    roomdialog.cpp \
    videoitem.cpp \
    fileitem.cpp

HEADERS  += wetalk.h \
    logindialog.h \
    chatdialog.h \
    useritem.h \
    IMToolBox.h \
    addfriend.h \
    roomdialog.h \
    videoitem.h \
    fileitem.h \
    fileitem.h

FORMS    += wetalk.ui \
    logindialog.ui \
    chatdialog.ui \
    useritem.ui \
    addfriend.ui \
    roomdialog.ui \
    videoitem.ui \
    fileitem.ui \
    fileitem.ui \
    fileitem.ui

RESOURCES += \
    resource.qrc

DISTFILES +=
