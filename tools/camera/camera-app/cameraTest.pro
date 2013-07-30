#-------------------------------------------------
#
# Project created by QtCreator 2013-05-29T12:53:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cameraTest
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    videowidget.cpp \
    v4l2thread.cpp \
    v4l2ctrls.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    v4l2thread.h \
    v4l2ctrls.h \
    v4l2utils.h \
    v4l2-dbg-bttv.h \
    v4l2-dbg-saa7134.h \
    v4l2-dbg-em28xx.h \
    v4l2-dbg-ac97.h \
    v4l2-dbg-tvp5150.h \
    v4l2-dbg.h \
    v4l2-chip-ident.h

FORMS    += mainwindow.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += libv4l2

unix: PKGCONFIG += libv4lconvert
