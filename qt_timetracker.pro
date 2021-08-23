QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++2a

#LIBS += -lX11

include(dbtool/dbtool.pri)
include(widgets/widgets.pri)

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dialogManual.cpp \
    dialogProject.cpp \
    main.cpp \
    mainwindow.cpp \
    mw_constructor.cpp \
    mw_initDatabase.cpp \
    mw_initTables.cpp \
    mw_listProjects.cpp \
    mw_timerTicks.cpp \
    mw_tracker.cpp \
    selectProject.cpp

HEADERS += \
    dialogManual.h \
    dialogProject.h \
    mainwindow.h \
    numFormat.h \
    selectProject.h

FORMS += \
    dialogManual.ui \
    dialogProject.ui \
    mainwindow.ui \
    selectProject.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
