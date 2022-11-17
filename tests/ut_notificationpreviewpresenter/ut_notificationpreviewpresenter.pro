include(../common.pri)
TARGET = ut_notificationpreviewpresenter
INCLUDEPATH += $$SRCDIR $$NOTIFICATIONSRCDIR $$UTILITYSRCDIR $$TOUCHSCREENSRCDIR $$SRCDIR/compositor $$DEVICESTATE $$SCREENLOCKSRCDIR
QT += qml quick dbus compositor

DEFINES += \
    LIPSTICK_UNIT_TEST_STUB

# unit test and unit
SOURCES += \
    ut_notificationpreviewpresenter.cpp \
    $$NOTIFICATIONSRCDIR/notificationpreviewpresenter.cpp \
    $$NOTIFICATIONSRCDIR/lipsticknotification.cpp \
    $$SCREENLOCKSRCDIR/screenlock.cpp \
    $$TOUCHSCREENSRCDIR/touchscreen.cpp \
    $$STUBSDIR/stubbase.cpp

# unit test and unit
HEADERS += \
    ut_notificationpreviewpresenter.h \
    $$NOTIFICATIONSRCDIR/notificationpreviewpresenter.h \
    $$NOTIFICATIONSRCDIR/notificationfeedbackplayer.h \
    $$NOTIFICATIONSRCDIR/notificationmanager.h \
    $$NOTIFICATIONSRCDIR/lipsticknotification.h \
    $$SCREENLOCKSRCDIR/screenlock.h \
    $$TOUCHSCREENSRCDIR/touchscreen.h \
    $$UTILITYSRCDIR/closeeventeater.h \
    $$COMPOSITORSRCDIR/lipstickcompositor.h \
    $$DEVICESTATE/displaystate.h \
    $$SRCDIR/homewindow.h \
    $$STUBSDIR/nemo-devicelock/devicelock.h
