greaterThan(QT_MAJOR_VERSION, 4): QT += printsupport axcontainer

HEADERS += \
    $$PWD/qcustomplot.h \
    $$PWD/qcustomchart.h \
    $$PWD/smoothcurvecreator.h

SOURCES += \
    $$PWD/qcustomplot.cpp \
    $$PWD/qcustomchart.cpp \
    $$PWD/smoothcurvecreator.cpp

RESOURCES += \
    $$PWD/chart.qrc

FORMS +=


