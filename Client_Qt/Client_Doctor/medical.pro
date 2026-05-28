QT += widgets network multimedia

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../Client/recorddetailwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    widget.cpp \
    loginwidget.cpp \
    dialog.cpp \
    audio.cpp \
    speech.cpp \
    http.cpp \
    doctorchatwidget.cpp \
    historydialog.cpp \
    settingswidget_doc.cpp

HEADERS += \
    ../Client/recorddetailwidget.h \
    mainwindow.h \
    widget.h \
    loginwidget.h \
    dialog.h \
    audio.h \
    speech.h \
    http.h \
    doctorchatwidget.h \
    historydialog.h \
    ../Client/resourcepaths.h \
    settingswidget_doc.h

FORMS += \
    ../Client/recorddetailwidget.ui \
    mainwindow.ui \
    widget.ui \
    loginwidget.ui \
    dialog.ui \
    doctorchatwidget.ui \
    historydialog.ui \
    settingswidget_doc.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
