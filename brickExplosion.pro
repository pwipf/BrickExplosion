#-------------------------------------------------
#
# Project created by QtCreator 2016-03-26T20:37:38
#
#-------------------------------------------------

QT       += opengl

INCLUDEPATH += $$PWD/include


CONFIG += console c++11

SOURCES += main.cpp\
        glwidget.cpp \
    mainwindow.cpp \
    mesh.cpp

HEADERS  += glwidget.h \
    mainwindow.h \
    mesh.h

RESOURCES += \
    shaders.qrc

FORMS += \
    mainwindow.ui


