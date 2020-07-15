QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

win32 {
    CONFIG += embed_manifest_exe
    QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\'
    QMAKE_CXXFLAGS += /utf-8
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -Ld:/boost_1_68_0/x64/lib64-msvc-14.1

INCLUDEPATH += d:\boost_1_68_0\x64

SOURCES += \
    logging.cpp \
    main.cpp \
    mainwindow.cpp \
    log_agent.cpp \
    jsoncpp.cpp \
    acceptor.cpp

HEADERS += \
    logging.h \
    mainwindow.h \
    log_agent.h \
    acceptor.h \
    json.h \
    json-forwards.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    farm_resource.qrc
