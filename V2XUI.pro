#-------------------------------------------------
#
# Project created by QtCreator 2018-08-28T08:32:57
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = V2XUI
TEMPLATE = app

MOC_DIR     = temp/moc
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj
DESTDIR     = $$PWD/./bin

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


include(./myNavigationWidget/myNavigationWidget.pri)
include(./myCVDisplayWidget/myCVDisplayWidget.pri)
include(./mySpliterWidget/mySpliterWidget.pri)
include(./myToastWidget/myToastWidget.pri)
include(./qcustomplot/qcustomplot.pri)
include(./myVideoLable/qVideoLable.pri)
include (qntp/qntp.pri)

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    appinit.cpp \
    workerudpreadobject.cpp \
    workerudpsendobject.cpp \
    workertcpobject.cpp \
    imagetool.cpp \
    dialogselectvideosourcedevice.cpp

HEADERS += \
        mainwindow.h \
    appinit.h \
    workerudpreadobject.h \
    workerudpsendobject.h \
    msghead.h \
    workertcpobject.h \
    imagetool.h \
    dialogselectvideosourcedevice.h

FORMS += \
        mainwindow.ui \
    dialogselectvideosourcedevice.ui

RESOURCES += \
    skin.qrc \
    staticrsrc.qrc
INCLUDEPATH += D:/Qt/opencv2.4.9/build/include \
               D:/Qt/opencv2.4.9/build/include/opencv \
               D:/Qt/opencv2.4.9/build/include/opencv2

LIBS += -L D:/Qt/opencv2.4.9/MinGW/lib/libopencv_*

#INCLUDEPATH += D:/Qt/opencv3.1.0/build/include \
#               D:/Qt/opencv3.1.0/build/include/opencv \
#               D:/Qt/opencv3.1.0/build/include/opencv2

#LIBS += -L D:/Qt/opencv3.1.0/MinGW/lib/libopencv_*

DISTFILES +=
