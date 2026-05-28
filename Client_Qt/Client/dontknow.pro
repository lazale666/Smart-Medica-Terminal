QT += widgets network multimedia texttospeech

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audio.cpp \
    dialog.cpp \
    doctordialog.cpp \
    doctorlistwidget.cpp \
    facerecognizewidget.cpp \
    http.cpp \
    loginwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    memberrechargewidget.cpp \
    menuwidget.cpp \
    medicalrecordwidget.cpp \
    recorddetailwidget.cpp \
    settingswidget.cpp \
    speech.cpp \
    widget.cpp

HEADERS += \
    audio.h \
    dialog.h \
    doctordialog.h \
    doctorlistwidget.h \
    facerecognizewidget.h \
    http.h \
    loginwidget.h \
    mainwindow.h \
    memberrechargewidget.h \
    menuwidget.h \
    medicalrecordwidget.h \
    recorddetailwidget.h \
    settingswidget.h \
    speech.h \
    widget.h

FORMS += \
    dialog.ui \
    doctordialog.ui \
    doctorlistwidget.ui \
    facerecognizewidget.ui \
    loginwidget.ui \
    mainwindow.ui \
    memberrechargewidget.ui \
    menuwidget.ui \
    medicalrecordwidget.ui \
    recorddetailwidget.ui \
    settingswidget.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
