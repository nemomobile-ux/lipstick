system(qdbusxml2cpp notifications/notificationmanager.xml -a notifications/notificationmanageradaptor -c NotificationManagerAdaptor -l NotificationManager -i notificationmanager.h)
system(qdbusxml2cpp screenlock/screenlock.xml -a screenlock/screenlockadaptor -c ScreenLockAdaptor -l ScreenLock -i screenlock.h)
system(qdbusxml2cpp screenshotservice.xml -a screenshotserviceadaptor -c ScreenshotServiceAdaptor -l ScreenshotService -i screenshotservice.h)
system(qdbusxml2cpp shutdownscreen.xml -a shutdownscreenadaptor -c ShutdownScreenAdaptor -l ShutdownScreen -i shutdownscreen.h)
system(qdbusxml2cpp net.connman.vpn.Agent.xml -a connmanvpnagent -c ConnmanVpnAgentAdaptor -l VpnAgent -i vpnagent.h)
system(qdbusxml2cpp -c ConnmanVpnProxy -p connmanvpnproxy net.connman.vpn.xml -i qdbusxml2cpp_dbus_types.h)
system(qdbusxml2cpp -c ConnmanManagerProxy -p connmanmanagerproxy net.connman.manager.xml -i qdbusxml2cpp_dbus_types.h)
system(qdbusxml2cpp -c ConnmanServiceProxy -p connmanserviceproxy net.connman.service.xml -i qdbusxml2cpp_dbus_types.h)

TEMPLATE = lib
TARGET = lipstick-qt5

DEFINES += LIPSTICK_BUILD_LIBRARY
DEFINES += VERSION=\\\"$${VERSION}\\\"
DEFINES += MESA_EGL_NO_X11_HEADERS
DEFINES += EGL_NO_X11

CONFIG += qt wayland-scanner c++11
PKGCONFIG += timed-qt5

INSTALLS = target ts_install engineering_english_install
target.path = $$[QT_INSTALL_LIBS]

QMAKE_STRIP = echo
OBJECTS_DIR = .obj
MOC_DIR = .moc

INCLUDEPATH += utilities touchscreen components xtools 3rdparty devicestate

#FOR BLUEZQT Wait https://invent.kde.org/frameworks/bluez-qt/-/merge_requests/14
INCLUDEPATH += /usr/include/KF5/BluezQt
LIBS += -lKF5BluezQt

include(compositor/compositor.pri)

PUBLICHEADERS += \
    utilities/qobjectlistmodel.h \
    utilities/closeeventeater.h \
    touchscreen/touchscreen.h \
    homeapplication.h \
    homewindow.h \
    lipstickglobal.h \
    lipsticksettings.h \
    lipstickdbus.h \
    lipstickqmlpath.h \
    components/launcheritem.h \
    components/launchermodel.h \
    components/launcherwatchermodel.h \
    components/launchermonitor.h \
    components/launcherdbus.h \
    components/launcherfoldermodel.h \
    notifications/notificationmanager.h \
    notifications/lipsticknotification.h \
    notifications/notificationlistmodel.h \
    notifications/notificationpreviewpresenter.h \
    usbmodeselector.h \
    localemanager.h \
    shutdownscreen.h \
    devicestate/displaystate.h \
    devicestate/devicestate.h \
    devicestate/thermal.h \
    vpnagent.h \
    connectivitymonitor.h

INSTALLS += publicheaderfiles dbus_policy
publicheaderfiles.files = $$PUBLICHEADERS
publicheaderfiles.path = /usr/include/lipstick-qt5
dbus_policy.files += lipstick.conf
dbus_policy.path = /etc/dbus-1/system.d

HEADERS += \
    $$PUBLICHEADERS \
    3rdparty/synchronizelists.h \
    3rdparty/dbus-gmain/dbus-gmain.h \
    bluetooth/bluetoothagent.h \
    bluetooth/bluetoothobexagent.h \
    notifications/notificationmanageradaptor.h \
    notifications/categorydefinitionstore.h \
    notifications/batterynotifier.h \
    notifications/notificationfeedbackplayer.h \
    notifications/androidprioritystore.h \
    screenlock/screenlock.h \
    screenlock/screenlockadaptor.h \
    touchscreen/touchscreen_p.h \
    volume/pulseaudiosinkinputmodel.h \
    volume/volumecontrol.h \
    volume/pulseaudiocontrol.h \
    lipstickapi.h \
    lipstickqmlpath.h \
    shutdownscreenadaptor.h \
    screenshotservice.h \
    screenshotserviceadaptor.h \
    qdbusxml2cpp_dbus_types.h \
    connmanvpnagent.h \
    connmanvpnproxy.h \
    connmanmanagerproxy.h \
    connmanserviceproxy.h \
    notifications/thermalnotifier.h \
    devicestate/devicestate_p.h \
    devicestate/displaystate_p.h \
    devicestate/ipcinterface_p.h \
    devicestate/thermal_p.h \
    logging.h \

SOURCES += \
    3rdparty/dbus-gmain/dbus-gmain.c \
    bluetooth/bluetoothagent.cpp \
    bluetooth/bluetoothobexagent.cpp \
    homeapplication.cpp \
    homewindow.cpp \
    lipsticksettings.cpp \
    lipstickqmlpath.cpp \
    utilities/qobjectlistmodel.cpp \
    utilities/closeeventeater.cpp \
    components/launcheritem.cpp \
    components/launchermodel.cpp \
    components/launcherwatchermodel.cpp \
    components/launchermonitor.cpp \
    components/launcherdbus.cpp \
    components/launcherfoldermodel.cpp \
    notifications/notificationmanager.cpp \
    notifications/notificationmanageradaptor.cpp \
    notifications/lipsticknotification.cpp \
    notifications/categorydefinitionstore.cpp \
    notifications/notificationlistmodel.cpp \
    notifications/notificationpreviewpresenter.cpp \
    notifications/batterynotifier.cpp \
    notifications/androidprioritystore.cpp \
    screenlock/screenlock.cpp \
    screenlock/screenlockadaptor.cpp \
    touchscreen/touchscreen.cpp \
    volume/pulseaudiosinkinputmodel.cpp \
    volume/volumecontrol.cpp \
    volume/pulseaudiocontrol.cpp \
    notifications/notificationfeedbackplayer.cpp \
    usbmodeselector.cpp \
    localemanager.cpp \
    shutdownscreen.cpp \
    shutdownscreenadaptor.cpp \
    vpnagent.cpp \
    connectivitymonitor.cpp \
    connmanvpnagent.cpp \
    connmanvpnproxy.cpp \
    connmanmanagerproxy.cpp \
    connmanserviceproxy.cpp \
    lipstickapi.cpp \
    screenshotservice.cpp \
    screenshotserviceadaptor.cpp \
    notifications/thermalnotifier.cpp \
    devicestate/displaystate.cpp \
    devicestate/devicestate.cpp \
    devicestate/thermal.cpp \
    devicestate/ipcinterface.cpp \
    logging.cpp \

CONFIG += link_pkgconfig mobility qt warn_on depend_includepath qmake_cache target_qt
CONFIG -= link_prl

PKGCONFIG += \
    dbus-1 \
    dsme_dbus_if \
    glib-2.0 \
    keepalive \
    libpulse \
    libpulse-mainloop-glib \
    libsystemd \
    mlite5 \
    mce \
    mce-qt5 \
    nemodevicelock \
    ngf-qt5 \
    systemsettings \
    thermalmanager_dbus_if \
    usb-moded-qt5

LIBS += -lrt -lEGL

packagesExist(contentaction5) {
    message("Using contentaction to launch applications")
    PKGCONFIG += contentaction5
    DEFINES += HAVE_CONTENTACTION
} else {
    PKGCONFIG += \
        gio-2.0 \
        gio-unix-2.0
    warning("contentaction doesn't exist; falling back to exec - this may not work so great")
}

packagesExist(sailfishusermanager) {
    DEFINES += HAVE_SAILFISHUSERMANAGER
}

QT += dbus xml qml quick sql gui gui-private sensors

QMAKE_CXXFLAGS += \
    -Wfatal-errors \
    -g \
    -fPIC \
    -fvisibility=hidden \
    -fvisibility-inlines-hidden \
    -Werror \
    -Wno-deprecated-copy

QMAKE_LFLAGS += \
    -pie \
    -rdynamic

QMAKE_CLEAN += \
    *.gcov \
    ./.obj/*.gcno

CONFIG += create_pc create_prl
QMAKE_PKGCONFIG_NAME = lib$$TARGET
QMAKE_PKGCONFIG_VERSION = $$VERSION
QMAKE_PKGCONFIG_DESCRIPTION = Library for creating QML desktops
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_INCDIR = $$publicheaderfiles.path
QMAKE_PKGCONFIG_DESTDIR = pkgconfig

pkgconfig.files = $$TARGET.pc
pkgconfig.path = $$target.path/pkgconfig
INSTALLS += pkgconfig

# translations
TS_FILE = $$OUT_PWD/lipstick.ts
EE_QM = $$OUT_PWD/lipstick_eng_en.qm
ts.commands += lupdate $$PWD -ts $$TS_FILE
ts.CONFIG += no_check_exist
ts.output = $$TS_FILE
ts.input = .
ts_install.files = $$TS_FILE
ts_install.path = /usr/share/translations/source
ts_install.CONFIG += no_check_exist

# should add -markuntranslated "-" when proper translations are in place (or for testing)
engineering_english.commands += lrelease -idbased $$TS_FILE -qm $$EE_QM
engineering_english.CONFIG += no_check_exist
engineering_english.depends = ts
engineering_english.input = $$TS_FILE
engineering_english.output = $$EE_QM
engineering_english_install.path = /usr/share/translations
engineering_english_install.files = $$EE_QM
engineering_english_install.CONFIG += no_check_exist


TRANSLATIONS = $$files(../i18n/$$TARGET.*.ts)

for (translation, TRANSLATIONS) {
    translation = $$basename(translation)
    QM_FILES += $$OUT_PWD/$$replace(translation, \\..*$, .qm)
}

updateqm.input = TRANSLATIONS
updateqm.output = $$OUT_PWD/${QMAKE_FILE_BASE}.qm
updateqm.commands = lrelease -idbased -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

qmfiles.files = $$QM_FILES
qmfiles.path = /usr/share/translations
qmfiles.CONFIG += no_check_exist

INSTALLS += qmfiles

QMAKE_EXTRA_TARGETS += ts engineering_english
PRE_TARGETDEPS += ts engineering_english

androidpriorities.files = androidnotificationpriorities
androidpriorities.path = /usr/share/lipstick/

INSTALLS += androidpriorities

include(notificationcategories/notificationcategories.pri)
