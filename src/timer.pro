#-------------------------------------------------
#
# Project created by QtCreator 2019-08-03T23:04:35
#
#-------------------------------------------------

QT       += core gui sql
QT      += network
QT      += multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

RC_ICONS = t.ico
VERSION = 0.9.5

TARGET = timer
TEMPLATE = app
QMAKE_TARGET_DESCRIPTION = "Timer"
# 版权信息
QMAKE_TARGET_COPYRIGHT = "Copyright 2021 Mr.Chang All rights reserved."
# 中文（简体）
RC_LANG = 0x0004

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        dialog.cpp

HEADERS += \
        dialog.h

FORMS += \
        dialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
