#-------------------------------------------------
#
# Project created by QtCreator 2013-02-13T13:33:00
#
# Zum kompilieren von QSerialDevice auf linux:
# sudo apt-get install libudev-dev
# in das verzeichnis ./serial wechseln und hier einen sprechenden build-ordner anlegen
# in diesen wechseln
#   hier:   qmake ../qserialdevice-qserialdevice/BuildLibrary.pro
#           make
# um ttyUSB0 ohne sudo öffnen zu können:
#   sudo adduser user_name dialout
#   dann neu einloggen - fertig
#
#-------------------------------------------------

QT       += core gui
QT       += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = waage2016
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    drawingarea.cpp \
    measurement.cpp \
    filter.cpp \
    settingsdialog.cpp

HEADERS  += widget.h \
    drawingarea.h \
    global.h \
    measurement.h \
    filter.h \
    settingsdialog.h

FORMS    += widget.ui \
    settingsdialog.ui

DISTFILES += \
    coeffs.txt

