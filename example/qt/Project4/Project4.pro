QT += core gui widgets

CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    dialogwindow.cpp \
    formwindow.cpp

HEADERS += \
    mainwindow.h \
    dialogwindow.h \
    formwindow.h

FORMS += \
    mainwindow.ui \
    dialogwindow.ui \
    formwindow.ui

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
